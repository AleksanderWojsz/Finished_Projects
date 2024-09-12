#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <fstream>
#include <poll.h>
#include <map>
#include <fcntl.h>
#include <csignal>
#include <netdb.h>
#include "utilities.h"

using std::cout;
using std::endl;
using std::string;
using std::map;
using std::vector;
using std::list;
using std::pair;

typedef struct Program_args {
    char* host;
    int32_t port;
    int32_t ip_version;
    char player_position;
    bool bot;
} Program_args;

void parse_args(int argc, char** argv, Program_args& program_args) {
    int32_t opt;

    program_args.host = nullptr;
    program_args.port = -1;
    program_args.ip_version = -1;
    program_args.player_position = 'X';
    program_args.bot = false;

    while ((opt = getopt(argc, argv, ":h:p:46NESWa")) != -1) {
        switch (opt) {
        case 'h':
            program_args.host = optarg;
            break;
        case 'p':
            program_args.port = read_port(optarg);
            if (program_args.port == 0) { // `read_port` sprawdza wszystkie inne niepoprawne przypadki
                throw error("0 is not a valid port number", errno);
            }
            break;
        case '4':
            program_args.ip_version = 4;
            break;
        case '6':
            program_args.ip_version = 6;
            break;
        case 'N':
        case 'E':
        case 'S':
        case 'W':
            program_args.player_position = (char)opt;
            break;
        case 'a':
            program_args.bot = true;
            break;
        default:
            throw error("Usage: ./kierki-klient -h <host> -p <port> [-4|-6] [-N|-E|-S|-W] [-a]", errno);
        }
    }

    // Czy nazwa pliku została podana i czy liczba poprawnych parametrów jest równa liczbie wszystkich parametrów
    if (program_args.host == nullptr || program_args.port == -1 ||
            program_args.player_position == 'X' || optind != argc) {
        throw error("Usage: ./kierki-klient -h <host> -p <port> [-4|-6] [-N|-E|-S|-W] [-a]", errno);
    }
}

void handle_user(vector<struct pollfd>& poll_descriptors, int32_t& last_correct_trick_number,
        list<pair<string, size_t>>& to_send, Player_game_status& player_game_status) {
    static ssize_t user_input_length = 0;
    static vector<char> user_input_buffer(BUFFER_SIZE);

    if (poll_descriptors[0].revents & (POLLIN)) { // Użytkownik coś napisał
        char read_character;
        ssize_t r;
        if ((r = read(STDIN_FILENO, &read_character, 1)) < 0) { throw error("function read()", errno); }
        user_input_buffer[user_input_length] = read_character;
        user_input_length += r;

        string user_message = string(&user_input_buffer[0], user_input_length);
        if (read_character == '\n') {
            user_input_length = 0;

            if (user_message == "cards\n") { // wyświetlenie listy kart na ręce;
                string s;
                for (const Card& card : player_game_status.get_cards()) {
                    s += card.to_string() += ", ";
                }
                cout << s.substr(0, s.size() - 2) << endl;

            } else if (user_message == "tricks\n") { // wyświetlenie listy lew wziętych w ostatniej rozgrywce w kolejności wzięcia
                for (const vector<Card>& cards_in_trick : player_game_status.tricks_taken_in_current_deal) {
                    string s;
                    for (const Card& card : cards_in_trick) {
                        s += card.to_string() += ", ";
                    }
                    cout << s.substr(0, s.size() - 2) << endl;
                }

            } else if (user_message.at(0) == '!') { // Jak zaczyna się od `!`, to wysyłam TRICK
                string chosen_card = user_message.substr(1, user_message.size() - 2);
                user_message = "TRICK" + std::to_string(last_correct_trick_number) + chosen_card + "\r\n";
                send_ANY_client(poll_descriptors, to_send, user_message);
                // Wysłaną kartę usunę ze swojej talii dopiero, jak dostanę TAKEN
            } else { // Wysyłam inną, nawet złą wiadomość
                user_message = user_message.substr(0, user_message.size() - 1) + "\r\n";
                send_ANY_client(poll_descriptors, to_send, user_message);
            }
        } else if (user_message.length() == BUFFER_SIZE) { // Ucinam wiadomość, bo ta nie mieści się w buforze
            user_input_length = 0;
        }
    }
}

void bot_send_TRICK(vector<struct pollfd>& poll_descriptors, list<pair<string, size_t>>& to_send,
                    Player_game_status& player_game_status, const Card& starting_card, int32_t current_TRICK) {
    vector<Card> my_cards = player_game_status.get_cards();
    Card chosen_card("x", 'x');
    bool found_card_in_starting_color = false;

    // Szukanie najmniejszej karty w kolorze, który został położony jako pierwszy
    for (const Card& card : my_cards) {
        if (card.color == starting_card.color) {
            if (!found_card_in_starting_color || card < chosen_card) {
                chosen_card = card;
                found_card_in_starting_color = true;
            }
        }
    }

    // Jeśli nie można dołożyć do koloru, to szukamy najmniejszej karty w dowolnym kolorze
    if (!found_card_in_starting_color) {
        chosen_card = my_cards[0];
        for (const Card& card : my_cards) {
            if (card < chosen_card) {
                chosen_card = card;
            }
        }
    }

    string message = "TRICK" + std::to_string(current_TRICK) + chosen_card.to_string() + "\r\n";
    to_send.emplace_back(message, message.length());
    poll_descriptors[1].events = POLLOUT | POLLIN;
}

void mark_all_messages_unexpected(map<string, bool>& is_message_expected) {
    for (string s : {"IAM", "BUSY", "DEAL", "TRICK", "WRONG", "TAKEN", "SCORE", "TOTAL"}) {
        is_message_expected[s] = false;
    }
}

bool handle_server(vector<struct pollfd>& poll_descriptors, int32_t server_fd, Player_game_status& player_game_status,
                   int32_t& last_correct_trick_number, list<pair<string, size_t>>& to_send, Program_args program_args,
                   bool& game_finished, map<string, bool>& is_message_expected) {
    static string taken_seats;
    static vector<Card> my_cards;
    static vector<Card> cards_on_table;
    static vector<Card> taken_cards;
    static vector<char> buffer(BUFFER_SIZE);
    static map<char, int32_t> deal_scores;
    static map<char, int32_t> total_scores;
    static ssize_t total_read = 0;
    static int32_t trick_number;
    static int32_t wrong_trick;
    static char who_takes;

    if (poll_descriptors[1].revents & (POLLIN | POLLERR | POLLHUP)) { // Serwer coś napisał
        ssize_t read;
        if ((read = recv(server_fd, &buffer[0] + total_read, 1, 0)) < 0) { throw error("function recv()", errno); }
        if (read == 0) { // Serwer się zakończył
            close(server_fd);
            return true;
        }
        total_read += read;
        string message = string(&buffer[0], total_read);

        if (message.length() >= 2 && message.at(message.length() - 2) == '\r' && message.at(message.length() - 1) == '\n') {
            if (is_message_expected.at("BUSY") && is_message_BUSY(message, taken_seats)) {
                mark_all_messages_unexpected(is_message_expected);
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); } else { print_BUSY(message); }
                close(server_fd);
                return true;
            } else if (is_message_expected.at("DEAL") && is_message_DEAL(message, my_cards)) {
                mark_all_messages_unexpected(is_message_expected);
                is_message_expected.at("TRICK") = true;
                is_message_expected.at("TAKEN") = true; // Możliwe po dołączeniu gracza w trakcie już rozpoczętej gry
                is_message_expected.at("WRONG") = true;
                player_game_status.set_cards(my_cards);
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); } else { print_DEAL(message); }
                last_correct_trick_number = 1;
            } else if (is_message_expected.at("TRICK") && is_message_TRICK(message, cards_on_table, trick_number)) {
                mark_all_messages_unexpected(is_message_expected);
                is_message_expected.at("WRONG") = true;
                is_message_expected.at("TRICK") = true;
                is_message_expected.at("TAKEN") = true;

                // Logi, w przeciwieństwie do komunikatów, wyświetlam zawsze.
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); }

                if (trick_number == last_correct_trick_number + 1) { // Potwierdzenie, że poprzedni wysłany TRICK był poprawny
                    last_correct_trick_number = last_correct_trick_number + 1;

                    if (!program_args.bot) {
                        print_TRICK(message, player_game_status);
                    } else {
                        bot_send_TRICK(poll_descriptors, to_send, player_game_status,
                                       cards_on_table.empty() ? Card("x", 'x') : cards_on_table.front(),
                                       last_correct_trick_number);
                    }
                } else if (trick_number == last_correct_trick_number || last_correct_trick_number == 1) { // Pierwszy lub powtórzony TRICK
                    last_correct_trick_number = trick_number;
                    if (!program_args.bot) {
                        print_TRICK(message, player_game_status);
                    } else {
                        bot_send_TRICK(poll_descriptors, to_send, player_game_status,
                                       cards_on_table.empty() ? Card("x", 'x') : cards_on_table.front(),
                                       last_correct_trick_number);
                    }
                }

            } else if (is_message_expected.at("TAKEN") && is_message_TAKEN(message, taken_cards, who_takes)) {
                mark_all_messages_unexpected(is_message_expected);
                is_message_expected.at("TRICK") = true;
                is_message_expected.at("SCORE") = true;
                is_message_expected.at("TOTAL") = true;
                is_message_expected.at("TAKEN") = true; // Możliwe po dołączeniu nowego klienta w trakcie już rozpoczętej gry
                is_message_expected.at("WRONG") = true;
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); } else { print_TAKEN(message); }

                for (const Card& card : taken_cards) { // Usuwam ze swojej talii kartę, którą położyłem
                    if (player_game_status.does_player_have_card(card)) {
                        player_game_status.remove_card(card);
                    }
                }

                // Zapisujemy, że wzięliśmy lewę.
                if (program_args.player_position == who_takes) {
                    player_game_status.tricks_taken_in_current_deal.push_back(taken_cards);
                }
            } else if (is_message_expected.at("SCORE") && is_message_SCORE(message, deal_scores)) {
                mark_all_messages_unexpected(is_message_expected);
                is_message_expected.at("TOTAL") = true;
                is_message_expected.at("DEAL") = true;
                is_message_expected.at("WRONG") = true;
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); } else { print_SCORE(message); }
            } else if (is_message_expected.at("TOTAL") && is_message_TOTAL(message, total_scores)) {
                mark_all_messages_unexpected(is_message_expected);
                is_message_expected.at("SCORE") = true;
                is_message_expected.at("DEAL") = true;
                is_message_expected.at("WRONG") = true;
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); } else { print_TOTAL(message); }
                game_finished = true;
            } else if (is_message_expected.at("WRONG") && is_message_WRONG(message, wrong_trick)) {
                mark_all_messages_unexpected(is_message_expected);
                is_message_expected.at("TRICK") = true;
                is_message_expected.at("WRONG") = true;
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); } else { print_WRONG(message); }
            } else { // Niepoprawną wiadomość jedynie wypisujemy
                if (program_args.bot) { print_log(message, server_fd, false, program_args.ip_version); }
            }

            total_read = 0;
        } else if (message.length() == BUFFER_SIZE) { // Ucinam wiadomość, bo ta nie mieści się w buforze
            print_log(message + "\r\n", server_fd, false, program_args.ip_version);
            total_read = 0;
        }
    }

    if (poll_descriptors[1].revents & POLLOUT) {
        ssize_t sent;
        size_t already_sent = 0;
        size_t remaining_to_send = to_send.front().first.length() - already_sent;
        if ((sent = send(server_fd, to_send.front().first.c_str() + already_sent, remaining_to_send, 0)) < 0) { throw error("function send()", errno); }

        to_send.front().second -= sent;
        if (to_send.front().second == 0) {
            if (program_args.bot) {
                print_log(to_send.front().first, server_fd, true, program_args.ip_version);
            }
            to_send.pop_front();

            if (to_send.empty()) {
                poll_descriptors[1].events = POLLIN;
            }
        }
    }

    return false;
}

int32_t connect_with_any(const char* host, uint16_t port, int32_t& server_fd) {
    struct addrinfo hints {};
    struct addrinfo* res;
    struct addrinfo* it;

    // Zamiana portu na napis
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%d", port);

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    hints.ai_family = AF_UNSPEC;

    int32_t ret = getaddrinfo(host, port_str, &hints, &res);
    if (ret != 0) { freeaddrinfo(res); throw error("function getaddrinfo()", errno); }

    for (it = res; it != nullptr; it = it->ai_next) {
        server_fd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (server_fd == -1) {
            continue;
        }

        if (connect(server_fd, it->ai_addr, it->ai_addrlen) == -1) {
            close(server_fd);
            continue;
        }

        break;
    }

    if (it == nullptr) {
        freeaddrinfo(res);
        throw error("Cannot connect to the server", errno);
    }

    int32_t ip_type = it->ai_family;
    freeaddrinfo(res);

    return ip_type;
}

int main(int argc, char* argv[]) {
    int32_t server_fd = -1;
    try {
        Program_args program_args;
        parse_args(argc, argv, program_args);

        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) { throw error("function signal()", errno); }

        if (program_args.ip_version == 4) {
            if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { throw error("function socket()", errno); }
            struct sockaddr_in server_address = get_server_address_ipv4(program_args.host, program_args.port);
            if (connect(server_fd, (const struct sockaddr*)&server_address, sizeof(server_address))) { throw error("function connect()", errno); }

        } else if (program_args.ip_version == 6) {
            if ((server_fd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) { throw error("function socket()", errno); }
            struct sockaddr_in6 server_address = get_server_address_ipv6(program_args.host, program_args.port);
            if (connect(server_fd, (const struct sockaddr*)&server_address, sizeof(server_address))) { throw error("function connect()", errno); }
        } else {
            int32_t ip_version = connect_with_any(program_args.host, program_args.port, server_fd);
            if (ip_version == AF_INET) {
                program_args.ip_version = 4;
            } else { // AF_INET6
                program_args.ip_version = 6;
            }
        }

        if (fcntl(server_fd, F_SETFL, O_NONBLOCK) < 0) { throw error("function fcntl()", errno); }

        int32_t last_correct_trick_number = 1;
        Player_game_status player_game_status;
        list<pair<string, size_t>> to_send;
        map<string, bool> is_message_expected;
        bool game_finished = false;

        vector<struct pollfd> poll_descriptors;
        poll_descriptors.push_back({STDIN_FILENO, POLLIN, 0}); // Użytkownik
        poll_descriptors.push_back({server_fd, POLLIN, 0}); // Serwer

        send_IAM(program_args.player_position, poll_descriptors, to_send);
        mark_all_messages_unexpected(is_message_expected);
        is_message_expected.at("BUSY") = true;
        is_message_expected.at("DEAL") = true;
        is_message_expected.at("WRONG") = true; // w teorii komunikat WRONG można dostać praktycznie w każdej sytuacji.
        // Np. klient wysyła IAM, następnie nieproszony wysyła TRICK i serwer najpierw reaguje na TRICK poprzez wysłanie WRONG
        // (w mojej implementacji serwera tak się nie stanie, ale klient potencjalnie może być łączony z innym serwerem).

        while (true) {
            int32_t poll_ret_val = poll(&poll_descriptors[0], poll_descriptors.size(), -1);
            if (poll_ret_val < 0) { throw error("poll() failed", errno); }
            else {

                if (!program_args.bot) {
                    handle_user(poll_descriptors, last_correct_trick_number, to_send, player_game_status);
                }

                if (handle_server(poll_descriptors, server_fd, player_game_status, last_correct_trick_number,
                                  to_send, program_args, game_finished, is_message_expected)) {
                    // Koniec gry
                    if (game_finished) {
                        return 0;
                    } else {
                        return 1;
                    }
                }
            }
        }

    } catch (error& e) {
        std::cerr << e.what() << std::endl;
        close(server_fd);
        return 1;
    }
}
