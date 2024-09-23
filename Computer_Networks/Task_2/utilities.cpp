#include <cstdint>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>
#include <cstdlib>
#include <climits>
#include <iostream>
#include <arpa/inet.h>
#include <iomanip>
#include <string>
#include <sys/socket.h>
#include <vector>
#include <fstream>
#include <poll.h>
#include <map>
#include <queue>
#include <regex>
#include "utilities.h"

using std::cout;
using std::endl;
using std::cerr;
using std::string;
using std::pair;
using std::regex;
using std::smatch;
using std::to_string;

/* Klasa Card */

Card::Card(string value, char color) : value(value), color(color) {}

bool Card::operator==(const Card& other) const {
    return color == other.color && value == other.value;
}

bool Card::operator<(const Card& other) const {
    return getNumericValue(value) < getNumericValue(other.value);
}

bool Card::operator>(const Card& other) const {
    return getNumericValue(value) > getNumericValue(other.value);
}

string Card::to_string() const {
    return value + color;
}

int32_t Card::getNumericValue(const string& value) {
    if (value == "2") return 2;
    if (value == "3") return 3;
    if (value == "4") return 4;
    if (value == "5") return 5;
    if (value == "6") return 6;
    if (value == "7") return 7;
    if (value == "8") return 8;
    if (value == "9") return 9;
    if (value == "10") return 10;
    if (value == "J") return 11;
    if (value == "Q") return 12;
    if (value == "K") return 13;
    if (value == "A") return 14;
    return -1;
}

/* Klasa ClientTimeout */

Waiting_client::Waiting_client(int32_t fd)
        : fd(fd), in_read_buffer(0), already_sent(0) {
    read_buffer.reserve(BUFFER_SIZE);
    message_to_send = "";
}

Waiting_clients::Waiting_clients(int32_t timeout_seconds)
        : timeout(timeout_seconds * 1000) {}

void Waiting_clients::add_client(int32_t client_descriptor) {
    clients.emplace(client_descriptor, Waiting_client(client_descriptor));
    clients.at(client_descriptor).added_time = std::chrono::high_resolution_clock::now();
}

int32_t Waiting_clients::get_time_until_client_timeout() {
    if (clients.empty()) {
        return -1;
    }

    auto now = std::chrono::high_resolution_clock::now();
    int32_t min_remaining_time = INT_MAX;

    for (const auto& client : clients) {
        int32_t elapsed = (int32_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - client.second.added_time).count();
        int32_t remaining_time = (int32_t)timeout.count() - elapsed;
        if (remaining_time < min_remaining_time) {
            min_remaining_time = remaining_time;
        }
    }

    return min_remaining_time > 0 ? min_remaining_time : 0;
}

int32_t Waiting_clients::return_earliest_client_fd() {
    auto now = std::chrono::high_resolution_clock::now();
    int32_t min_remaining_time = std::numeric_limits<int32_t>::max();
    int32_t earliest_client_fd = -1;

    for (const auto& client : clients) {
        int32_t elapsed = (int32_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - client.second.added_time).count();
        int32_t remaining_time = (int32_t)timeout.count() - elapsed;
        if (remaining_time < min_remaining_time) {
            min_remaining_time = remaining_time;
            earliest_client_fd = client.first;
        }
    }

    return earliest_client_fd;
}

Waiting_client* Waiting_clients::get_waiting_client(int32_t client_descriptor) {
    auto it = clients.find(client_descriptor);
    if (it != clients.end()) {
        return &it->second;
    }
    return nullptr;
}

void Waiting_clients::remove_client(int32_t client_descriptor) {
    clients.erase(client_descriptor);
}

/* Klasa Table */

Player::Player(int32_t fd)
        : fd(fd), in_read_buffer(0), is_asked_for_a_card(false) {
    read_buffer.reserve(BUFFER_SIZE);
}

Table::Table(int32_t timeout_seconds) : timeout(timeout_seconds), game_started(false),
    current_deal(0), current_trick(1), timer_running(false) {

    elapsed_time = std::chrono::steady_clock::duration::zero();

    for (char p : {'N', 'E', 'S', 'W'}) {
        total_scores[p] = 0;
        current_deal_scores[p] = 0;
        player_game_status.insert({p, Player_game_status()});
    }
}

bool Table::all_seats_taken() const {
    return players.size() == 4;
}

bool Table::is_given_seat_empty(char position) const {
    return players.count(position) == 0;
}

string Table::get_taken_seats() const {
    string s;
    for (char l : {'N', 'E', 'S', 'W'}) {
        s += (is_given_seat_empty(l) ? "" : string(1, l));
    }
    return s;
}

void Table::add_player(int32_t fd, char position) {
    players.erase(position); // emplace wstawia element, tylko jeśli nie było go wcześniej
    players.emplace(position, Player(fd));
}

void Table::remove_player(char position) {
    players.erase(position);
}

Player* Table::find_player_by_position(char position) {
    auto it = players.find(position);
    if (it != players.end()) {
        return &it->second;
    }
    throw error("find_player_by_position()", errno);
}

void Table::player_asked_for_card(char position) {
    auto player = find_player_by_position(position);
    if (player) {
        player->is_asked_for_a_card = true;
        player->asked_for_a_card_time = get_current_time_in_game();
    }
}

void Table::player_gave_card(char position, const Card& card) {
    auto player = find_player_by_position(position);
    if (player) {
        player->is_asked_for_a_card = false;
        cards_on_the_table.insert({position, card});
    }
}

void Table::start_timer() {
    if (!timer_running) {
        start_time = std::chrono::steady_clock::now();
        timer_running = true;
    }
}

void Table::stop_timer() {
    if (timer_running) {
        elapsed_time += std::chrono::steady_clock::now() - start_time;
        timer_running = false;
    }
}

std::chrono::steady_clock::duration Table::get_current_time_in_game() {
    if (timer_running) {
        return elapsed_time + (std::chrono::steady_clock::now() - start_time);
    } else {
        return elapsed_time;
    }
}

int32_t Table::time_until_timeout() {
    int32_t min_time_left_ms = INT_MAX;

    for (pair<const char, Player>& player : players) {
        if (player.second.is_asked_for_a_card) {
            auto duration_since_asked = std::chrono::duration_cast<std::chrono::milliseconds>(get_current_time_in_game() - player.second.asked_for_a_card_time);
            int32_t time_left_ms = int32_t(timeout.count() * 1000 - duration_since_asked.count());
            if (time_left_ms < min_time_left_ms) {
                min_time_left_ms = time_left_ms;
            }
        }
    }

    min_time_left_ms = std::max(0, min_time_left_ms); // Czasami mogła wyjść mała ujemna liczba
    return (min_time_left_ms == INT_MAX) ? -1 : min_time_left_ms; // Jak nie znaleziono żadnego czasu to -1
}

string Table::cards_on_table_to_string(char starting_position) {
    string result;
    for (int32_t i = 0; i < 4; i++) {
        if (cards_on_the_table.count(starting_position) == 0) {
            break;
        }
        result += cards_on_the_table.at(starting_position).to_string();
        starting_position = next_position(starting_position);
    }

    return result;
}

size_t Table::number_of_cards_on_table() const {
    return cards_on_the_table.size();
}

bool Table::does_player_give_card(char position) const {
    return cards_on_the_table.contains(position);
}

void Table::clear_table_from_cards() {
    cards_on_the_table.clear();
}

char Table::player_with_highest_card_of_given_color(char color) {
    Card highest_card("x", 'x');
    char result = 'x';

    for (pair<char, Card> pair : cards_on_the_table) {
        char position = pair.first;
        Card card = pair.second;

        if (card.color == color && (result == 'x' || card > highest_card)) {
            highest_card = card;
            result = position;
        }
    }

    return result;
}

/*
    1. nie brać lew, za każdą wziętą lewę dostaje się 1 punkt;
    2. nie brać kierów, za każdego wziętego kiera dostaje się 1 punkt;
    3. nie brać dam, za każdą wziętą damę dostaje się 5 punktów;
    4. nie brać panów (waletów i króli), za każdego wziętego pana dostaje się 2 punkty;
    5. nie brać króla kier, za jego wzięcie dostaje się 18 punktów;
    6. nie brać siódmej i ostatniej lewy, za wzięcie każdej z tych lew dostaje się po 10 punktów;
    7. rozbójnik, punkty dostaje się za wszystko wymienione powyżej.
*/
char Table::update_scores() {
    Round_description current_round = rounds_descriptions.at(current_deal);
    char starting_color = cards_on_the_table.at(current_trick_starting_position).color;
    char loser = player_with_highest_card_of_given_color(starting_color);

    switch (current_round.game_type) {
    case 1:
        current_deal_scores[loser] += 1;
        total_scores[loser] += 1;
        break;
    case 2:
        for (pair<char, Card> p : cards_on_the_table) {
            Card c = p.second;
            if (c.color == 'H') {
                current_deal_scores[loser] += 1;
                total_scores[loser] += 1;
            }
        }
        break;
    case 3:
        for (pair<char, Card> p : cards_on_the_table) {
            Card c = p.second;
            if (c.value == "Q") {
                current_deal_scores[loser] += 5;
                total_scores[loser] += 5;
            }
        }
        break;
    case 4:
        for (pair<char, Card> p : cards_on_the_table) {
            Card c = p.second;
            if (c.value == "J" || c.value == "K") {
                current_deal_scores[loser] += 2;
                total_scores[loser] += 2;
            }
        }
        break;
    case 5:
        for (pair<char, Card> p : cards_on_the_table) {
            Card c = p.second;
            if (c.value == "K" && c.color == 'H') {
                current_deal_scores[loser] += 18;
                total_scores[loser] += 18;
            }
        }
        break;
    case 6:
        if (current_trick == 7 || current_trick == 13) {
            current_deal_scores[loser] += 10;
            total_scores[loser] += 10;
        }
        break;
    case 7:
        current_deal_scores[loser] += 1;
        total_scores[loser] += 1;
        for (pair<char, Card> p : cards_on_the_table) {
            Card c = p.second;
            if (c.color == 'H') {
                current_deal_scores[loser] += 1;
                total_scores[loser] += 1;
            }
            if (c.value == "Q") {
                current_deal_scores[loser] += 5;
                total_scores[loser] += 5;
            }
            if (c.value == "J" || c.value == "K") {
                current_deal_scores[loser] += 2;
                total_scores[loser] += 2;
            }
            if (c.value == "K" && c.color == 'H') {
                current_deal_scores[loser] += 18;
                total_scores[loser] += 18;
            }
        }
        if (current_trick == 7 || current_trick == 13) {
            current_deal_scores[loser] += 10;
            total_scores[loser] += 10;
        }
        break;
    }

    return loser;
}

/* Pozostałe funkcje */

uint16_t read_port(const string& s) {
    char* endptr;
    size_t port = strtoul(s.c_str(), &endptr, 10);
    if ((port == ULONG_MAX && errno == ERANGE) || *endptr != 0 || port > UINT16_MAX) {
        throw error(s + " is not a valid port number", errno);
    }
    return (uint16_t)port;
}

struct sockaddr_in get_server_address_ipv4(char const* host, uint16_t port) {
    struct sockaddr_in addr {};
    struct addrinfo hints {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Pozwala na używanie 'localhost' zamiast np. 127.0.0.1
    struct addrinfo* address_result;
    int32_t ret = getaddrinfo(host, nullptr, &hints, &address_result);
    if (ret != 0) { throw error("function getaddrinfo(): " + string(gai_strerror(ret)), errno); }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ((struct sockaddr_in*)(address_result->ai_addr))->sin_addr.s_addr;
    addr.sin_port = htons(port);

    freeaddrinfo(address_result);

    return addr;
}

/*
 * Jeżeli host jest pustym napisem, to host = INADDR_ANY
 */
struct sockaddr_in6 get_server_address_ipv6(char const* host, uint16_t port) {
    struct sockaddr_in6 addr {};
    if (strlen(host) != 0) {
        struct addrinfo hints {};
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        struct addrinfo* address_result;
        int32_t ret = getaddrinfo(host, nullptr, &hints, &address_result);
        if (ret != 0) { throw error("function getaddrinfo(): " + string(gai_strerror(ret)), errno); }

        addr = *((struct sockaddr_in6*)(address_result->ai_addr));
        addr.sin6_port = htons(port);

        freeaddrinfo(address_result);
    } else {
        addr.sin6_family = AF_INET6;
        addr.sin6_addr = in6addr_any;
        addr.sin6_port = htons(port);
    }

    return addr;
}

bool is_a_number(const string& s) {
    for (char i : s) {
        if (!isdigit(i)) {
            return false;
        }
    }

    return true;
}

void print_log(const string& s, int32_t fd, bool sending, int32_t ip_version) {
    if (ip_version == 4) {
        print_log_ipv4(s, fd, sending);
    } else {
        print_log_ipv6(s, fd, sending);
    }
}

void print_log_ipv6(string s, int32_t fd, bool sending) {
    // Podczas odbierania ostatnich wiadomości w rozgrywce serwer może być już rozłączony, stąd (przynajmniej klient)
    // potrzebuje zapamiętać adres serwera, żeby móc go wypisać.
    static map<int32_t, pair<struct sockaddr_in6, struct sockaddr_in6>> address_map;
    struct sockaddr_in6 server_addr {};
    struct sockaddr_in6 client_addr {};

    if (address_map.find(fd) == address_map.end()) {
        socklen_t server_len = sizeof(server_addr);
        socklen_t client_len = sizeof(client_addr);

        if (getsockname(fd, (struct sockaddr*)&server_addr, &server_len) < 0) { throw error("function getsockname() error", errno); }
        if (getpeername(fd, (struct sockaddr*)&client_addr, &client_len) < 0) { throw error("function getpeername() error", errno); }

        address_map[fd] = std::make_pair(server_addr, client_addr);
    } else {
        server_addr = address_map[fd].first;
        client_addr = address_map[fd].second;
    }

    char server_ip[INET6_ADDRSTRLEN];
    char client_ip[INET6_ADDRSTRLEN];

    // Zamiana na formę tekstową
    if (inet_ntop(AF_INET6, &server_addr.sin6_addr, server_ip, sizeof(server_ip)) == nullptr) { throw error("function inet_ntop() error", errno); }
    if (inet_ntop(AF_INET6, &client_addr.sin6_addr, client_ip, sizeof(client_ip)) == nullptr) { throw error("function inet_ntop() error", errno); }

    int32_t server_port = ntohs(server_addr.sin6_port);
    int32_t client_port = ntohs(client_addr.sin6_port);

    if (sending) {
        cout << "[" << server_ip << ":" << server_port << "," << client_ip << ":" << client_port << ",";
    } else {
        cout << "[" << client_ip << ":" << client_port << "," << server_ip << ":" << server_port << ",";
    }

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms.time_since_epoch());
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);

    cout << std::put_time(std::localtime(&time_t_now), "%FT%T") << '.'
              << std::setfill('0') << std::setw(3) << (value.count() % 1000) << "] ";

    cout << s;
    std::fflush(stdout);
}

void print_log_ipv4(string s, int32_t fd, bool sending) {
    // Podczas odbierania ostatnich wiadomości w rozgrywce serwer może być już rozłączony, stąd (przynajmniej klient)
    // potrzebuje zapamiętać adres serwera, żeby móc go wypisać.
    static map<int32_t, pair<struct sockaddr_in, struct sockaddr_in>> address_map;
    struct sockaddr_in server_addr {};
    struct sockaddr_in client_addr {};

    if (address_map.find(fd) == address_map.end()) {
        socklen_t server_len = sizeof(server_addr);
        socklen_t client_len = sizeof(client_addr);

        if (getsockname(fd, (struct sockaddr*)&server_addr, &server_len) < 0) { throw error("function getsockname() error", errno); }
        if (getpeername(fd, (struct sockaddr*)&client_addr, &client_len) < 0) { throw error("function getpeername() error", errno); }

        address_map[fd] = std::make_pair(server_addr, client_addr);
    } else {
        server_addr = address_map[fd].first;
        client_addr = address_map[fd].second;
    }

    char server_ip[INET_ADDRSTRLEN];
    char client_ip[INET_ADDRSTRLEN];

    // Zamiana na formę tekstową
    if (inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, sizeof(server_ip)) == nullptr) { throw error("function inet_ntop() error", errno); }
    if (inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip)) == nullptr) { throw error("function inet_ntop() error", errno); }

    int32_t server_port = ntohs(server_addr.sin_port);
    int32_t client_port = ntohs(client_addr.sin_port);

    if (sending) {
        cout << "[" << server_ip << ":" << server_port << "," << client_ip << ":" << client_port << ",";
    } else {
        cout << "[" << client_ip << ":" << client_port << "," << server_ip << ":" << server_port << ",";
    }

    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto value = std::chrono::duration_cast<std::chrono::milliseconds>(now_ms.time_since_epoch());
    std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);

    cout << std::put_time(std::localtime(&time_t_now), "%FT%T") << '.'
              << std::setfill('0') << std::setw(3) << (value.count() % 1000) << "] ";

    cout << s;
}

vector<Card> string_to_cards(string s) {
    vector<Card> cards;
    for (size_t i = 0; i < s.length(); i += 2) {
        if (s[i] == '1') {
            string value = "10";
            char color = s[i + 2];
            cards.emplace_back(value, color);
            i++;
        } else {
            string value = string(1, s[i]);
            char color = s[i + 1];
            cards.emplace_back(value, color);
        }
    }
    return cards;
}

bool is_message_BUSY(const string& s, string& seats) {
    regex pattern("^BUSY([NESW]{0,4})\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 2) {
            seats = matches[1];
            return true;
        }
    }

    return false;
}

bool is_message_DEAL(const string& s, vector<Card>& cards) {
    regex pattern("^DEAL[1-7][NESW](((10|[2-9JQKA])[CDHS]){13})\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 4) {
            cards = string_to_cards(matches[1]);
            return true;
        }
    }

    return false;
}

bool is_message_TRICK(const string& s, vector<Card>& cards, int32_t& trick_number) {
    regex pattern("^TRICK([0-9]+)(((10|[2-9JQKA])[CDHS])*)\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 5) {
            trick_number = stoi(matches[1].str());
            cards = string_to_cards(matches[2].str());
            return true;
        }
    }

    return false;
}

bool is_message_TAKEN(const string& s, vector<Card>& cards, char& who_takes) {
    regex pattern("^TAKEN[0-9]+(((10|[2-9JQKA])[CDHS]){4})([NESW])\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 5) {
            cards = string_to_cards(matches[1]);
            who_takes = matches[4].str()[0];
            return true;
        }
    }

    return false;
}

bool is_message_SCORE(const string& s, map<char, int32_t>& scores) {
    regex pattern("^SCORE([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 5) {
            for (int32_t i = 1; i <= 4; ++i) {
                char seat = matches[i].str()[0];
                int32_t score = stoi(matches[i].str().substr(1));
                scores[seat] = score;
            }
            return true;
        }
    }

    return false;
}

bool is_message_TOTAL(const string& s, map<char, int32_t>& scores) {
    regex pattern("^TOTAL([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 5) {
            for (int32_t i = 1; i <= 4; ++i) {
                char seat = matches[i].str()[0];
                int32_t score = stoi(matches[i].str().substr(1));
                scores[seat] = score;
            }
            return true;
        }
    }

    return false;
}

bool is_message_WRONG(const string& s, int32_t& trick_number) {
    regex pattern("^WRONG([0-9]{1,2})\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 2) {
            trick_number = stoi(matches[1].str());
            return true;
        }
    }

    return false;
}

void print_WRONG(const string& s) {
    regex pattern("^WRONG([0-9]{1,2})\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        string trick_number = matches[1].str();
        cout << "Wrong message received in trick " << trick_number << "." << endl;
    }
}

void print_BUSY(const string& s) {
    regex pattern("^BUSY([NESW]{0,4})\\r\\n$");
    smatch matches;
    if (regex_match(s, matches, pattern)) {
        string busy_places = matches[1].str();
        cout << "Place busy, list of busy places received: ";
        for (size_t i = 0; i < busy_places.size(); ++i) {
            if (i != 0) cout << ", ";
            cout << busy_places[i];
        }
        cout << "." << endl;
    }
}

void print_DEAL(const string& s) {
    regex pattern("^DEAL([1-7])([NESW])(((10|[2-9JQKA])[CDHS]){13})\\r\\n$");
    smatch matches;
    if (regex_match(s, matches, pattern)) {
        string deal_type = matches[1].str();
        char starting_place = matches[2].str()[0];
        vector<Card> cards = string_to_cards(matches[3].str());
        cout << "New deal " << deal_type << ": staring place " << starting_place << ", your cards: ";
        for (size_t i = 0; i < cards.size(); ++i) {
            if (i != 0) cout << ", ";
            cout << cards[i].to_string();
        }
        cout << "." << endl;
    }
}

void print_TAKEN(const string& s) {
    regex pattern("^TAKEN([0-9]+)(((10|[2-9JQKA])[CDHS]){4})([NESW])\\r\\n$");
    smatch matches;
    if (regex_match(s, matches, pattern)) {
        string trick_number = matches[1].str();
        vector<Card> cards = string_to_cards(matches[2].str());
        char winner_place = matches[5].str()[0];
        cout << "A trick " << trick_number << " is taken by " << winner_place << ", cards ";
        for (size_t i = 0; i < cards.size(); ++i) {
            if (i != 0) cout << ", ";
            cout << cards[i].to_string();
        }
        cout << "." << endl;
    }
}

void print_SCORE(const string& s) {
    regex pattern("^SCORE([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)\\r\\n$");
    smatch matches;
    if (regex_match(s, matches, pattern)) {
        cout << "The scores are:\n";
        for (int32_t i = 1; i <= 4; ++i) {
            string score_str = matches[i].str();
            char seat = score_str[0];
            int32_t score = stoi(score_str.substr(1));
            cout << seat << " | " << score << endl;
        }
    }
}

void print_TOTAL(const string& s) {
    regex pattern("^TOTAL([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)([NESW][0-9]+)\\r\\n$");
    smatch matches;
    if (regex_match(s, matches, pattern)) {
        cout << "The total scores are:\n";
        for (int32_t i = 1; i <= 4; ++i) {
            string score_str = matches[i].str();
            char seat = score_str[0];
            int32_t score = stoi(score_str.substr(1));
            cout << seat << " | " << score << endl;
        }
    }
}

void print_TRICK(const string& s, Player_game_status& player_game_status) {
    regex pattern("^TRICK([0-9]+)(((10|[2-9JQKA])[CDHS])*)\\r\\n$");
    smatch matches;
    if (regex_match(s, matches, pattern)) {
        string trick_number = matches[1].str();
        vector<Card> cards = string_to_cards(matches[2].str());
        cout << "Trick: (" << trick_number << ") ";
        for (size_t i = 0; i < cards.size(); ++i) {
            if (i != 0) cout << ", ";
            cout << cards[i].to_string();
        }
        cout << "\n";

        cards = player_game_status.get_cards();
        cout << "Available: ";
        for (size_t i = 0; i < cards.size(); ++i) {
            if (i != 0) cout << ", ";
            cout << cards[i].to_string();
        }
        cout << endl;
    }
}

void send_SCORE(Table& table, vector<struct pollfd>& poll_descriptors, char destination_position) {
    string message = "SCORE";
    for (char p : {'N', 'E', 'S', 'W'}) {
        message += string(1, p) + to_string(table.current_deal_scores.at(p));
    }
    message += "\r\n";

    Player* player = table.find_player_by_position(destination_position);
    player->messages_to_send.emplace_back(message, 0);
    poll_descriptors[position_to_index(destination_position)].events = POLLOUT | POLLIN;
}

void send_TOTAL(Table& table, vector<struct pollfd>& poll_descriptors, char destination_position) {
    string message = "TOTAL";
    for (char p : {'N', 'E', 'S', 'W'}) {
        message += string(1, p) + to_string(table.total_scores.at(p));
    }
    message += "\r\n";

    Player* player = table.find_player_by_position(destination_position);
    player->messages_to_send.emplace_back(message, 0);
    poll_descriptors[position_to_index(destination_position)].events = POLLOUT | POLLIN;
}

void send_WRONG(Table& table, vector<struct pollfd>& poll_descriptors, char destination_position) {
    string message = "WRONG" + to_string(table.current_trick) + "\r\n";

    Player* player = table.find_player_by_position(destination_position);
    player->messages_to_send.emplace_back(message, 0);
    poll_descriptors[position_to_index(destination_position)].events = POLLOUT | POLLIN;
}

void send_DEAL(Table& table, vector<struct pollfd>& poll_descriptors, char position) {
    Round_description round = table.rounds_descriptions.at(table.current_deal);

    string message = "DEAL" + to_string(round.game_type) + string(1, round.first_player);
    for (const Card& card : round.cards.at(position)) {
        message += card.to_string();
    }
    message += "\r\n";

    Player* player = table.find_player_by_position(position);
    player->messages_to_send.emplace_back(message, 0);
    poll_descriptors[position_to_index(position)].events = POLLOUT | POLLIN;
}

void send_TRICK_server(Table& table, vector<struct pollfd>& poll_descriptors, char starting_position) {
    char position = table.whose_turn;
    string message = "TRICK" + to_string(table.current_trick) + table.cards_on_table_to_string(starting_position) + "\r\n";
    Player* player = table.find_player_by_position(position);
    player->messages_to_send.emplace_back(message, 0);
    poll_descriptors[position_to_index(position)].events = POLLOUT | POLLIN;
}

void send_BUSY(int32_t fd, Waiting_clients& waiting_clients, Table& table) {
    waiting_clients.get_waiting_client(fd)->message_to_send = "BUSY" + table.get_taken_seats() + "\r\n";
    waiting_clients.get_waiting_client(fd)->already_sent = 0;
}

void send_TAKEN_to_everyone(Table& table, vector<struct pollfd>& poll_descriptors, char loser, char starting_player) {
    string cards;
    for (int32_t i = 0; i < 4; i++) {
        cards += table.cards_on_the_table.at(starting_player).to_string();
        starting_player = next_position(starting_player);
    }

    string message = "TAKEN" + to_string(table.current_trick) + cards + string(1, loser) + "\r\n";
    table.taken_messages_from_current_deal.push_back(message);

    for (char p : {'N', 'E', 'S', 'W'}) {
        Player* player = table.find_player_by_position(p);
        player->messages_to_send.emplace_back(message, 0);
        poll_descriptors[position_to_index(p)].events = POLLOUT | POLLIN;
    }
}

void send_IAM(char position, vector<struct pollfd>& poll_descriptors, list<pair<string, size_t>>& to_send) {
    string message = "IAM" + string(1, position) + "\r\n";
    to_send.emplace_back(message, message.length());
    poll_descriptors[1].events = POLLOUT | POLLIN;
}

void send_ANY_client(vector<struct pollfd>& poll_descriptors, list<pair<string, size_t>>& to_send, string& message) {
    to_send.emplace_back(message, message.length());
    poll_descriptors[1].events = POLLOUT | POLLIN;
}

void send_ANY_server(Table& table, vector<struct pollfd>& poll_descriptors,
        char destination_position, const string& message) {
    Player* player = table.find_player_by_position(destination_position);
    player->messages_to_send.emplace_back(message, 0); // emplace_back tworzy obiekt w miejscu
    poll_descriptors[position_to_index(destination_position)].events = POLLOUT | POLLIN;
}

bool is_message_IAM(string s, char* position) {
    if (s.length() == strlen("IAMN\r\n")) {
        *position = s[3];
        regex pattern("^IAM[NSEW]\\r\\n$");
        return regex_match(s, pattern);
    }

    return false;
}

bool is_message_TRICK(const string& s, Card& card, int32_t& trick_number) {
    regex pattern("^TRICK([0-9]{1,2})(((10|[2-9JQKA])[CDHS])*)\\r\\n$");
    smatch matches;

    if (regex_match(s, matches, pattern)) {
        if (matches.size() == 5) {
            trick_number = stoi(matches[1].str());
            card = string_to_cards(matches[3]).front();
            return true;
        }
    }

    return false;
}

int32_t position_to_index(char c) {
    if (c == 'N') return 1;
    else if (c == 'E') return 2;
    else if (c == 'S') return 3;
    else return 4;
}

char next_position(char position) {
    char positions[] = {'N', 'E', 'S', 'W'};

    for (int32_t i = 0; i < 4; ++i) {
        if (positions[i] == position) {
            return positions[(i + 1) % 4];
        }
    }
    throw error("Function next_position(). Incorrect position.", errno);
}

void remove_fd_from_poll_array(vector<struct pollfd>& poll_descriptors, int32_t fd) {
    int32_t index = 0;
    for (struct pollfd client : poll_descriptors) {
        if (client.fd == fd) {
            poll_descriptors.erase(poll_descriptors.begin() + index);
            break;
        }
        index++;
    }
}

void print_error(const string& msg, int32_t errnum) {
    cerr << "ERROR: " + msg + ". Errno: " + to_string(errnum) + " - " + strerror(errnum) << endl;
}
