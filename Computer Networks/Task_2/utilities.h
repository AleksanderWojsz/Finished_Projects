#ifndef UTILITIES_H
#define UTILITIES_H

// Duży rozmiar bufora wynika ze wpisu na forum,
// że wiadomości (nawet niepoprawne) należy w miarę możliwości wypisywać w całości
#define BUFFER_SIZE 2048

#define QUEUE_LENGTH 10

#include <stdexcept>
#include <iostream>
#include <string>
#include <iostream>
#include <map>
#include <chrono>
#include <limits>
#include <climits>
#include <vector>
#include <cstring>
#include <list>

using std::vector;
using std::map;
using std::string;
using std::pair;
using std::list;

class Card {
public:
    string value;
    char color;

    Card(string value, char color);

    bool operator==(const Card& other) const;
    bool operator<(const Card& other) const;
    bool operator>(const Card& other) const;

    [[nodiscard]] string to_string() const;

    static int32_t getNumericValue(const string& value);
};

struct Waiting_client {
    int32_t fd;
    std::chrono::time_point<std::chrono::high_resolution_clock> added_time;
    vector<char> read_buffer;
    size_t in_read_buffer;
    string message_to_send;
    size_t already_sent;

    explicit Waiting_client(int32_t fd);
};

class Waiting_clients {
private:
    map<int32_t, Waiting_client> clients;
    std::chrono::milliseconds timeout;

public:
    explicit Waiting_clients(int32_t timeout_seconds);

    void add_client(int32_t client_descriptor);
    int32_t get_time_until_client_timeout();
    int32_t return_earliest_client_fd();
    Waiting_client* get_waiting_client(int32_t client_descriptor);
    void remove_client(int32_t client_descriptor);
};

class Player_game_status {
public:
    vector<Card> players_cards;
    vector<vector<Card>> tricks_taken_in_current_deal;

    void set_cards(vector<Card> cards) {
        players_cards = std::move(cards);
    }

    [[nodiscard]] vector<Card> get_cards() const {
        return players_cards;
    }

    bool remove_card(const Card& card) {
        for (auto it = players_cards.begin(); it != players_cards.end(); ++it) {
            if (*it == card) {
                players_cards.erase(it);
                return true;
            }
        }
        return false;
    }

    bool does_player_have_card(const Card& card) {
        for (const Card& players_card : players_cards) {
            if (players_card == card) {
                return true;
            }
        }
        return false;
    }

    bool does_player_have_any_card_in_color(char color) {
        for (const Card& players_card : players_cards) {
            if (players_card.color == color) {
                return true;
            }
        }
        return false;
    }
};

typedef struct Round_description {
    int32_t game_type;
    char first_player;
    map<char, vector<Card>> cards;
} Round_description;

struct Player {
    int32_t fd;
    vector<char> read_buffer;
    size_t in_read_buffer;
    std::list<std::pair<string, size_t>> messages_to_send; // <wiadomość, liczba już wysłanych bajtów z tej wiadomości>
    std::chrono::steady_clock::duration asked_for_a_card_time;
    bool is_asked_for_a_card;

    explicit Player(int32_t fd);
};

class Table {
public:
    map<char, Player> players;
    map<char, Card> cards_on_the_table; // Karta na stole jest przypisana do miejsca zamiast do gracza, żeby karta została po tym, jak gracz wyszedł.
    map<char, Player_game_status> player_game_status;
    std::chrono::seconds timeout;
    vector<Round_description> rounds_descriptions;
    map<char, int32_t> current_deal_scores;
    map<char, int32_t> total_scores;
    vector<string> taken_messages_from_current_deal;
    bool game_started;
    int32_t current_deal;
    int32_t current_trick;
    char whose_turn;
    char current_trick_starting_position;

    explicit Table(int32_t timeout_seconds);
    string cards_on_table_to_string(char starting_position);
    [[nodiscard]] size_t number_of_cards_on_table() const;
    [[nodiscard]] bool does_player_give_card(char position) const;
    void clear_table_from_cards();
    char update_scores();

    [[nodiscard]] bool all_seats_taken() const;
    [[nodiscard]] bool is_given_seat_empty(char position) const;
    [[nodiscard]] string get_taken_seats() const;
    void add_player(int32_t fd, char position);
    void remove_player(char position);
    Player* find_player_by_position(char position);
    void player_asked_for_card(char position);
    void player_gave_card(char position, const Card& card);
    int32_t time_until_timeout();

    void start_timer();
    void stop_timer();
    std::chrono::steady_clock::duration get_current_time_in_game();

private:
    char player_with_highest_card_of_given_color(char color);
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::duration elapsed_time;
    bool timer_running;
};

class error : public std::runtime_error {
public:
    error(const string& msg, int32_t errnum)
        : std::runtime_error(format_message(msg, errnum)) {
    }

private:
    static string format_message(const string& msg, int32_t errnum) {
        if (errnum != 0) {
            return "ERROR: " + msg + ". Errno: " + std::to_string(errnum) + " - " + strerror(errnum);
        } else {
            return "ERROR: " + msg;
        }
    }
};

struct sockaddr_in get_server_address_ipv4(char const* host, uint16_t port);
struct sockaddr_in6 get_server_address_ipv6(char const* host, uint16_t port);
uint16_t read_port(const string& s);
bool is_a_number(const string& s);
void print_log(const string&, int32_t fd, bool sending, int32_t ip_version);
void print_log_ipv4(string s, int32_t fd, bool sending);
void print_log_ipv6(string s, int32_t fd, bool sending);
void print_error(const string& msg, int32_t errnum);
vector<Card> string_to_cards(string s);
void remove_fd_from_poll_array(vector<struct pollfd>& poll_descriptors, int32_t fd);
int32_t position_to_index(char c);
char next_position(char position);

bool is_message_BUSY(const string& s, string& seats);
bool is_message_DEAL(const string& s, vector<Card>& cards);
bool is_message_TRICK(const string& s, vector<Card>& cards, int32_t& trick_number);
bool is_message_TAKEN(const string& s, vector<Card>& cards, char& who_takes);
bool is_message_SCORE(const string& s, map<char, int32_t>& scores);
bool is_message_TOTAL(const string& s, map<char, int32_t>& scores);
bool is_message_WRONG(const string& s, int32_t& trick_number);
void print_WRONG(const string& s);
void print_BUSY(const string& s);
void print_DEAL(const string& s);
void print_TAKEN(const string& s);
void print_SCORE(const string& s);
void print_TOTAL(const string& s);
void print_TRICK(const string& s, Player_game_status& player_game_status);

bool is_message_TRICK(const string& s, Card& card, int32_t& trick_number);
bool is_message_IAM(string s, char* position);
void send_SCORE(Table& table, vector<struct pollfd>& poll_descriptors, char destination_position);
void send_TOTAL(Table& table, vector<struct pollfd>& poll_descriptors, char destination_position);
void send_WRONG(Table& table, vector<struct pollfd>& poll_descriptors, char destination_position);
void send_DEAL(Table& table, vector<struct pollfd>& poll_descriptors, char position);
void send_TRICK_server(Table& table, vector<struct pollfd>& poll_descriptors, char starting_position);
void send_BUSY(int32_t fd, Waiting_clients& waiting_clients, Table& table);
void send_TAKEN_to_everyone(Table& table, vector<struct pollfd>& poll_descriptors, char loser, char starting_player);
void send_IAM(char position, vector<struct pollfd>& poll_descriptors, list<pair<string, size_t>>& to_send);
void send_ANY_client(vector<struct pollfd>& poll_descriptors, list<pair<string, size_t>>& to_send, string& message);
void send_ANY_server(Table& table, vector<struct pollfd>& poll_descriptors, char destination_position, const string& message);

#endif // UTILITIES_H
