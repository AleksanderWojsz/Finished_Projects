#include <cstdlib>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <list>
#include <vector>
#include <fstream>
#include <poll.h>
#include <map>
#include <queue>
#include <fcntl.h>
#include "utilities.h"

using std::string;
using std::map;
using std::list;
using std::vector;
using std::priority_queue;

typedef struct Program_args {
    int32_t port;
    char* filename;
    int32_t timeout;
} Program_args;

void parse_args(int32_t argc, char** argv, Program_args& program_args) {
    int32_t opt;

    program_args.port = 0;
    program_args.timeout = 5;
    program_args.filename = nullptr;

    while ((opt = getopt(argc, argv, ":p:f:t:")) != -1) {
        switch (opt) {
        case 'p':
            program_args.port = read_port(optarg);
            break;
        case 'f':
            program_args.filename = optarg;
            break;
        case 't':
            if (!is_a_number(optarg)) { throw error("Negative timeout", errno); }
            program_args.timeout = atoi(optarg);
            if (program_args.timeout < 0) { throw error("Negative timeout", errno); }
            break;
        default:
            throw error("Usage: ./kierki-serwer [-p <port>] -f <file> [-t <timeout>]", errno);
        }
    }

    // Czy nazwa pliku została podana i czy liczba poprawnych parametrów jest równa liczbie wszystkich parametrów
    if (program_args.filename == nullptr || optind != argc) {
        throw error("Usage: ./kierki-serwer [-p <port>] -f <file> [-t <timeout>]", errno);
    }
}

vector<Round_description> read_rounds_descriptions(const string& filename) {
    std::ifstream inputFile(filename);

    if (!inputFile.is_open()) { throw error("Could not open rounds descriptions file", errno); }

    string line;
    vector<Round_description> rounds_descriptions;
    while (getline(inputFile, line)) { // Wczytanie opisu pojedyńczego rozdania
        Round_description round;

        // Wczytanie typu rozdania i zaczynającego klienta
        round.game_type = int32_t(line[0]) - int32_t('0');
        round.first_player = line[1];

        // Wczytanie kart dla każdego klienta
        for (char player : {'N', 'E', 'S', 'W'}) {
            getline(inputFile, line);

            for (size_t i = 0; i < line.size(); i += 2) {
                Card card(string(1, line[i]), line[i + 1]);
                if (line[i] == '1') { // Jak wartość karty to 10
                    card.value = "10";
                    card.color = line[i + 2];
                    i++;
                }
                round.cards[player].push_back(card);
            }
        }

        rounds_descriptions.push_back(round);
    }

    // fstream to obiekt RAII, więc na koniec sam by się zamknął, ale chcę dodatkowo sprawdzić ewentualny błąd.
    inputFile.close();
    if (inputFile.is_open()) { throw error("Could not close rounds descriptions file", errno); }

    return rounds_descriptions;
}

void timeout_while_waiting_for_IAM(vector<struct pollfd>& poll_descriptors, Waiting_clients& waiting_clients) {
    int32_t finished_fd = waiting_clients.return_earliest_client_fd();
    close(finished_fd);
    waiting_clients.remove_client(finished_fd);
    remove_fd_from_poll_array(poll_descriptors, finished_fd);
}

void timeout_while_waiting_for_TRICK(vector<struct pollfd>& poll_descriptors, Table& table) {
    send_TRICK_server(table, poll_descriptors, table.current_trick_starting_position);
    table.player_asked_for_card(table.whose_turn);
}

bool can_player_give_this_card(Table& table, char position, Card& card, char starting_position) {
    if (position != table.whose_turn || !table.player_game_status.at(position).does_player_have_card(card)) {
        // Gracz dał kartę, a nie jest jego kolej lub dał kartę, której nie ma.
        return false;
    } else if (position != starting_position) {
        char starting_color = table.cards_on_the_table.at(starting_position).color;
        if (card.color != starting_color &&
            table.player_game_status.at(position).does_player_have_any_card_in_color(starting_color)) {
            // Gracz nie zaczynał rozdania i nie dołożył karty do koloru, mimo że mógł.
            return false;
        }
    }

    return true;
}

void accept_new_connections(int32_t server_fd, vector<struct pollfd>& poll_descriptors, Waiting_clients& waiting_clients) {
    if (poll_descriptors[0].revents & POLLIN) {
        struct sockaddr_in client_address {};
        socklen_t address_len = sizeof(client_address);
        int32_t client_fd;
        if ((client_fd = accept(server_fd, (struct sockaddr*)&client_address, &address_len)) < 0) { throw error("function accept()", errno); }

        if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) { throw error("function fcntl()", errno); }

        waiting_clients.add_client(client_fd);

        poll_descriptors.push_back({client_fd, POLLIN, 0});
    }
}

void handle_clients_who_want_to_play(vector<struct pollfd>& poll_descriptors,
        Waiting_clients& waiting_clients, Table& table) {

    for (int32_t i = 5; i < (int32_t)poll_descriptors.size(); i++) {
        if (poll_descriptors[i].revents & (POLLIN | POLLERR | POLLHUP)) { // Któryś z połączonych, niebędących przy stole, klientów coś napisał
            int32_t fd = poll_descriptors[i].fd;

            Waiting_client* waiting_client = waiting_clients.get_waiting_client(fd);
            ssize_t read = recv(fd, &waiting_client->read_buffer[0] + waiting_client->in_read_buffer, 1, 0); // Czytam po jednym znaku
            waiting_client->in_read_buffer += read;

            if (read <= 0) { // Klient się zakończył lub jest związany nim błąd
                print_error("in handle_clients_who_want_to_play(). Function recv()", errno);
                close(fd);
                waiting_clients.remove_client(fd);
                poll_descriptors.erase(poll_descriptors.begin() + i);
                i--;
            } else {
                string message = std::string(&waiting_client->read_buffer[0], waiting_client->in_read_buffer);

                if (message.length() >= 2 && message.at(message.length() - 2) == '\r' && message.at(message.length() - 1) == '\n') { // Udało się odczytać wiadomość
                    waiting_client->in_read_buffer = 0;

                    char position;
                    if (is_message_IAM(message, &position)) {
                        print_log_ipv6(message, waiting_client->fd, false);

                        if (table.is_given_seat_empty(position)) {
                            // Usunięcie gracza z oczekujących i dodanie do grających
                            poll_descriptors.erase(poll_descriptors.begin() + i);
                            i--;
                            waiting_clients.remove_client(fd);
                            table.add_player(fd, position);

                            // Jak, po dodaniu tego gracza, wszystkie miejsca są zajęte,
                            // to zaczynam słuchać wiadomości od wszystkich czterech graczy
                            if (table.all_seats_taken()) {
                                table.start_timer(); // Czas w grze zaczyna płynąć
                                int32_t index = 1;
                                for (char p : {'N', 'E', 'S', 'W'}) {
                                    if (!table.players.at(p).messages_to_send.empty()) {
                                        poll_descriptors.at(index) = {table.players.at(p).fd, POLLOUT | POLLIN, 0};
                                    } else {
                                        poll_descriptors.at(index) = {table.players.at(p).fd, POLLIN, 0};
                                    }
                                    index++;
                                }
                            }

                            // Klient dołączył w trakcie gry
                            if (table.game_started) {
                                send_DEAL(table, poll_descriptors, position);

                                // Wysłanie dotychczas rozegranych lew za pomocą TAKEN
                                for (const string& taken_message : table.taken_messages_from_current_deal) {
                                    send_ANY_server(table, poll_descriptors, position, taken_message);
                                }

                                // Jeśli jest kolej dołączającego gracza, to dodatkowo wysyłamy mu TRICK
                                if (table.whose_turn == position) {
                                    send_TRICK_server(table, poll_descriptors, table.current_trick_starting_position);
                                    table.player_asked_for_card(table.whose_turn);
                                }
                            } else {
                                poll_descriptors.at(position_to_index(position)) = {fd, POLLIN, 0};
                            }

                        } else { // Pozycja zajęta lub gra w trakcie
                            send_BUSY(fd, waiting_clients, table);
                            poll_descriptors[i].events = POLLOUT | POLLIN;
                        }

                    } else { // Ktoś wysłał niepoprawne IAM
                        print_log_ipv6(message, waiting_client->fd, false);
                        close(fd);
                        waiting_clients.remove_client(fd);
                        remove_fd_from_poll_array(poll_descriptors, fd);
                        i--;
                    }
                } else if (message.length() == BUFFER_SIZE) { // Ucinam wiadomość, bo ta nie mieści się w buforze
                    print_log_ipv6(message + "\r\n", waiting_client->fd, false);
                    waiting_client->in_read_buffer = 0;
                }
            }
        }

        if (poll_descriptors[i].revents & POLLOUT) { // Jest miejsce w buforze do pisania
            int32_t fd = poll_descriptors[i].fd;
            Waiting_client* client = waiting_clients.get_waiting_client(fd);

            ssize_t sent;
            string message_to_send = client->message_to_send;
            size_t already_sent = client->already_sent;
            if ((sent = send(fd, message_to_send.c_str() + already_sent, message_to_send.length() - already_sent, 0)) < 0) { throw error("function send()", errno); }
            client->already_sent += sent;

            if (client->already_sent == message_to_send.length()) { // Po wysłaniu całej wiadomości BUSY zamykamy połączenie
                print_log_ipv6(message_to_send, client->fd, true);
                close(fd);
                waiting_clients.remove_client(fd);
                remove_fd_from_poll_array(poll_descriptors, fd);
                i--;
            }
        }
    }
}

void handle_players_in_game(vector<struct pollfd>& poll_descriptors, Table& table) {
    char positions[] =  {'N', 'E', 'S', 'W'};
    for (int32_t i = 1; i <= 4; i++) {
        char position = positions[i - 1];

        if (poll_descriptors[i].revents & (POLLIN | POLLERR | POLLHUP)) { // Gracz coś napisał
            Player* player = table.find_player_by_position(position);

            ssize_t read = recv(player->fd, &player->read_buffer[0] + player->in_read_buffer, 1, 0);
            player->in_read_buffer += read;

            if (read <= 0) { // Klient się zakończył lub jest związany nim błąd
                close(player->fd);
                table.remove_player(position);

                if (table.game_started) {
                    // Jak jakiś gracz się rozłączył, to zaczynam całkowicie ignorować wszystkich grających.
                    for (int32_t j = 1; j <= 4; j++) {
                        poll_descriptors[j] = {-1, POLLIN, 0};
                    }
                    table.stop_timer(); // Czas w grze się zatrzymuje
                } else { // Zanim gra się zacznie, jak jeden klient się zakończy, to i tak słuchamy od innych klientów.
                    poll_descriptors[i] = {-1, POLLIN, 0};
                }

                return;
            } else {
                string message;
                message = string(&player->read_buffer[0], player->in_read_buffer);

                if (message.length() >= 2 && message.at(message.length() - 2) == '\r' && message.at(message.length() - 1) == '\n') { // Udało się coś odczytać
                    player->in_read_buffer = 0;
                    Card card("x", 'x');
                    int32_t trick_number;
                    if (is_message_TRICK(message, card, trick_number)) {
                        print_log_ipv6(message, player->fd, false);

                        if (trick_number == table.current_trick &&
                                can_player_give_this_card(table, position, card, table.current_trick_starting_position)) { // Poprawny TRICK
                            table.player_gave_card(position, card);
                            table.player_game_status.at(position).remove_card(card);
                        } else { // Niepoprawny TRICK
                            send_WRONG(table, poll_descriptors, position);
                        }
                    } else { // Błędny komunikat inny niż położenie karty
                        print_log_ipv6(message, player->fd, false);
                        close(player->fd);
                        table.remove_player(position);

                        if (table.game_started) {
                            // Jak gracz się rozłączył, to zaczynam całkowicie ignorować wszystkich grających.
                            for (int32_t j = 1; j <= 4; j++) {
                                poll_descriptors[j] = {-1, POLLIN, 0};
                            }
                            table.stop_timer(); // Czas w grze się zatrzymuje
                        } else { // Zanim gra się zacznie, to jak jeden klient się zakończy, to i tak słuchamy od innych klientów.
                            poll_descriptors[i] = {-1, POLLIN, 0};
                        }
                    }
                } else if (message.length() == BUFFER_SIZE) { // Ucinam wiadomość, bo ta nie mieści się w buforze
                    print_log_ipv6(message + "\r\n", player->fd, false);
                    player->in_read_buffer = 0;
                }
            }
        }

        // Można pisać do gracza
        if (poll_descriptors[i].revents & POLLOUT) {
            Player* player = table.find_player_by_position(position);
            int32_t fd = player->fd;

            ssize_t sent;
            size_t already_sent = player->messages_to_send.front().second;
            string message_to_send = player->messages_to_send.front().first;

            if ((sent = send(fd, message_to_send.c_str() + already_sent, message_to_send.length() - already_sent, 0)) < 0) { throw error("function send()", errno); }
            player->messages_to_send.front().second += sent;

            // Wysłana cała jedna wiadomość
            if (player->messages_to_send.front().second == message_to_send.length()) {
                print_log_ipv6(message_to_send, player->fd, true);
                player->messages_to_send.pop_front();

                // Wszystkie wiadomości wysłane
                if (player->messages_to_send.empty()) {
                    poll_descriptors[i].events = POLLIN;
                }
            }
        }
    }
}

/*
 * Zwraca czy koniec gry
 */
bool manage_game(Table& table, vector<struct pollfd>& poll_descriptors) {
    // Rozpoczęcie pierwszego rozdania w pierwszej turze
    if (!table.game_started) {
        table.game_started = true;
        table.whose_turn = table.rounds_descriptions.at(0).first_player;
        table.current_trick_starting_position = table.whose_turn;

        // Wysłanie DEAL i TRICK do pierwszego gracza. Przypisanie graczom kart, które powinni mieć.
        for (char p : {'N', 'E', 'S', 'W'}) {
            table.player_game_status.at(p).set_cards(table.rounds_descriptions.at(table.current_deal).cards.at(p));
            send_DEAL(table, poll_descriptors, p);
        }
        send_TRICK_server(table, poll_descriptors, table.current_trick_starting_position); // Rozpoczęcie pierwszego rozdania
        table.player_asked_for_card(table.whose_turn);
    }

    // Pozostałe rozdania
    if (table.does_player_give_card(table.whose_turn)) {
        // gracz, na którego czekamy dał kartę
        if (table.number_of_cards_on_table() < 4) {
            table.whose_turn = next_position(table.whose_turn);
            send_TRICK_server(table, poll_descriptors, table.current_trick_starting_position); // Kontynuacja lewy
            table.player_asked_for_card(table.whose_turn);
        } else { // Wszyscy gracze dali kartę (teraz nawet jak gracz się rozłączy, to jeszcze w tej funkcji jego karta jest na stole)

            // Liczenie punktów i wysłanie TAKEN do każdego z graczy
            char loser = table.update_scores();
            send_TAKEN_to_everyone(table, poll_descriptors, loser, table.current_trick_starting_position);
            table.current_trick_starting_position = loser;

            table.current_trick++;
            table.clear_table_from_cards();

            if (table.current_trick < 14) {
                table.whose_turn = loser;
                send_TRICK_server(table, poll_descriptors, table.current_trick_starting_position); // Rozpoczęcie nowej lewy
                table.player_asked_for_card(table.whose_turn);
            } else {
                // Koniec rozdania
                for (char p : {'N', 'E', 'S', 'W'}) {
                    send_SCORE(table, poll_descriptors, p);
                    send_TOTAL(table, poll_descriptors, p);
                }

                // Zerować punkty można tylko po wysłaniu wiadomości do wszystkich
                for (char p : {'N', 'E', 'S', 'W'}) {
                    // Resetowanie punktacji rozdania
                    table.current_deal_scores[p] = 0;
                }

                table.current_trick = 1;
                table.current_deal++;
                if (table.current_deal == (int32_t)table.rounds_descriptions.size()) {
                    return true; // Koniec gry
                }

                table.taken_messages_from_current_deal.clear();
                table.whose_turn = table.rounds_descriptions.at(table.current_deal).first_player;
                table.current_trick_starting_position = table.whose_turn;

                // Wysłanie DEAL do wszystkich i TRICK do pierwszego gracza. Przypisanie graczom kart, które powinni mieć.
                for (char p : {'N', 'E', 'S', 'W'}) {
                    table.player_game_status.at(p).set_cards(table.rounds_descriptions.at(table.current_deal).cards.at(p));
                    send_DEAL(table, poll_descriptors, p);
                }
                send_TRICK_server(table, poll_descriptors, table.current_trick_starting_position); // Rozpoczęcie nowego rozdania
                table.player_asked_for_card(table.whose_turn);
            }
        }
    }

    return false;
}

/*
 * Kończy wysyłanie wszystkich wiadomości
 */
void close_game(vector<struct pollfd>& poll_descriptors, Waiting_clients& waiting_clients, Table& table) {
    close(poll_descriptors[0].fd); // Nie można już łączyć się z serwerem
    poll_descriptors[0].fd = -1;
    // Usunięcie tych, do których nic nie wysyłamy
    for (int32_t i = 5; i < (int32_t)poll_descriptors.size(); i++) {
        int32_t fd = poll_descriptors.at(i).fd;
        Waiting_client* waiting_client = waiting_clients.get_waiting_client(fd);
        if (waiting_client->message_to_send.empty()) {
            close(fd);
            waiting_clients.remove_client(fd);
            remove_fd_from_poll_array(poll_descriptors, fd);
            i--;
        }
    }

    int32_t remaining_players = 4;

    while (remaining_players != 0 || poll_descriptors.size() != 5) {
        int32_t poll_ret_val = poll(&poll_descriptors[0], poll_descriptors.size(), -1);
        if (poll_ret_val < 0) { throw error("poll() failed", errno); }
        else {
            // Kończy wysyłanie wiadomości do klientów nie w grze (BUSY). Po wysłaniu funkcja sama zamyka połączenie z klientem i go usuwa
            handle_clients_who_want_to_play(poll_descriptors, waiting_clients, table);

            // Kończy wysyłanie wiadomości do klientów w grze
            handle_players_in_game(poll_descriptors, table);
            // Sprawdzenie, czy któremuś z graczy wysłaliśmy już wszystko
            for (char p : {'N', 'E', 'S', 'W'}) {
                int32_t index = position_to_index(p);
                if (table.find_player_by_position(p)->messages_to_send.empty()) {
                    remaining_players--;
                    close(poll_descriptors[index].fd);
                    poll_descriptors[index].fd = -1;
                }
            }
        }
    }
}

void game(int32_t server_fd, Program_args program_args, vector<struct pollfd>& poll_descriptors) {
    Waiting_clients waiting_clients(program_args.timeout);
    Table table(program_args.timeout);
    table.rounds_descriptions = read_rounds_descriptions(program_args.filename);
    if (table.rounds_descriptions.empty()) {
        return;
    }

    for (int32_t i = 0; i < 4; i++) { // Na słuchanie od kolejno N, E, S, W
        poll_descriptors.push_back({-1, POLLIN, 0});
    }

    while (true) {
        int32_t timeout;
        bool timeout_type = false; // true = IAM_timeout, false = TRICK_timeout
        int32_t IAM_timeout = waiting_clients.get_time_until_client_timeout();
        int32_t TRICK_timeout = table.time_until_timeout();

        if (!table.all_seats_taken()) { // Jak nie ma któregoś z graczy, to uniemożliwiam timeout związany z niepołożeniem TRICK
            TRICK_timeout = -1;
        }

        if (IAM_timeout != -1 && TRICK_timeout != -1) {
            if (IAM_timeout < TRICK_timeout) {
                timeout = IAM_timeout;
                timeout_type = true;
            } else {
                timeout = TRICK_timeout;
                timeout_type = false;
            }
        } else if (IAM_timeout != -1) {
            timeout = IAM_timeout;
            timeout_type = true;
        } else if (TRICK_timeout != -1) {
            timeout = TRICK_timeout;
            timeout_type = false;
        } else {
            timeout = -1;
        }

        int32_t poll_ret_val = poll(&poll_descriptors[0], poll_descriptors.size(), timeout);
        if (poll_ret_val < 0) { throw error("poll() failed", errno); }
        else if (poll_ret_val == 0) {
            if (timeout_type) {
                timeout_while_waiting_for_IAM(poll_descriptors, waiting_clients);
            } else {
                timeout_while_waiting_for_TRICK(poll_descriptors, table);
            }
        } else { // poll_ret_val > 0

            // Ktoś nowy chce nawiązać połączenie
            accept_new_connections(server_fd, poll_descriptors, waiting_clients);

            // Odbieranie i wysyłanie wiadomości do tych, którzy chcą dołączyć do rozgrywki
            handle_clients_who_want_to_play(poll_descriptors, waiting_clients, table);

            // Odbieranie i wysyłanie wiadomości do graczy (modyfikuje obiekt table w tym liczbę graczy przy stole)
            if (table.all_seats_taken() || !table.game_started) {
                handle_players_in_game(poll_descriptors, table);
            }

            // Przeprowadzanie gry, ale tylko jak są wszyscy gracze
            if (table.all_seats_taken() && manage_game(table, poll_descriptors)) {
                // Koniec gry
                break;
            }
        }
    }

    close_game(poll_descriptors, waiting_clients, table);
}

int main(int argc, char* argv[]) {
    int32_t server_fd;
    vector<struct pollfd> poll_descriptors;
    try {
        Program_args program_args;
        parse_args(argc, argv, program_args);

        struct sockaddr_in6 server_address = get_server_address_ipv6("", program_args.port);

        if ((server_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) { throw error("function socket()", errno); }
        poll_descriptors.push_back({server_fd, POLLIN, 0});

        int32_t option = 0;
        if (setsockopt(server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &option, sizeof(option)) < 0) { throw error("function setsockopt()", errno); }

        if (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) { throw error("function bind()", errno); }

        if (listen(server_fd, QUEUE_LENGTH) < 0) { throw error("function listen()", errno); }

        game(server_fd, program_args, poll_descriptors);

    } catch (error& e) {
        std::cerr << e.what() << std::endl;
        for (struct pollfd p : poll_descriptors) {
            close(p.fd);
        }
        return 1;
    }

    return 0;
}
