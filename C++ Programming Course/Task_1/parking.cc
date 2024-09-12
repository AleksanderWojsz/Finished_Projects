/*
 * Zadanie 1, Kurs C++.
 * Aleksander Wojsz, Tytus Walężak.
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <regex>
#include <queue>
#include <unordered_map>

// Liczba minut od 8.00 do 20.00 (zakładamy, że dzień trwa od 8.00 do 20.00).
#define MINUTES_IN_A_DAY 721
#define MINUTES_IN_AN_HOUR 60
#define MINUTES_IN_EIGHT_HOURS 480
#define MINUTES_IN_TWELVE_HOURS 720
#define MINUTES_IN_TWENTY_HOURS 1200
#define TEN_MINUTES 10
#define TWENTY_HOURS 20
#define EIGHT_HOURS 8
#define SIXTY_MINUTES 60
#define ZERO_MINUTES 0

using namespace std;

/*
 * Komparator ustawiający pary nierosnąco według
 * liczby na drugiej pozycji każdej z par.
 */
class Queue_comparator {
public:
    bool operator()(const pair<string, int32_t> &pair_1,
        const pair<string, int32_t> &pair_2) {

        return pair_1.second <= pair_2.second;
    }
};

/*
 * Zamienia napis w formacie 'hh.mm' lub 'h.mm'
 * na parę liczb <godzina, minuty>, którą daje w wyniku.
 */
pair<int32_t, int32_t> extract_hours_and_minutes(const string &time) {
    istringstream stream(time);
    string read_time;
    vector<int32_t > result;

    // Dzieli podany czas na godziny i minuty.
    // W tej kolejności umieszcza je w 'result'.
    while (getline(stream, read_time, '.')) {
        result.push_back(stoi(read_time));
    }

    return make_pair(result[0], result[1]);
}

/*
 * Zamienia czas podany w formacie 'hh.mm' lub 'h.mm' na
 * liczbę minut od godziny 8.00.
 *
 * Dla godzin poza przedziałem 8.00 - 20.00 daje '-1'.
 * Dla błędnych wartości godzin lub minut (np. 10.61) daje '-1'.
 *
 * Dodatkowo do wyniku może dodać
 * liczbę minut w 'day_number' dniach (domyślnie zero).
 */
int32_t time_to_minutes(const string &time, const int32_t day_number = 0) {

    pair<int32_t, int32_t> hours_and_minutes =
        extract_hours_and_minutes(time);

    // Czy podany czas jest w przedziale 8.00 - 20.00.
    if (hours_and_minutes.first > TWENTY_HOURS ||
        hours_and_minutes.first < EIGHT_HOURS ||
        (hours_and_minutes.first == TWENTY_HOURS &&
        hours_and_minutes.second > ZERO_MINUTES) ||
        hours_and_minutes.second >= SIXTY_MINUTES ||
        hours_and_minutes.second < ZERO_MINUTES) {

        return -1;
    }

    // Chcemy policzyć minuty od godziny 8.00,
    // więc wynik zmniejszamy o 'MINUTES_IN_EIGHT_HOURS'.
    return hours_and_minutes.first * MINUTES_IN_AN_HOUR -
        MINUTES_IN_EIGHT_HOURS + hours_and_minutes.second +
        day_number * MINUTES_IN_A_DAY;
}

/*
 * Dzieli napis 'input' na mniejsze napisy. Wynik umieszcza w 'splitted_line'.
 *
 * 'input' rozdzielany jest tylko w miejscach pojawienia się
 * dowolnej liczby białych znaków.
 */
void split_line(vector<string>& splitted_line, const string &input) {
    stringstream stream(input);
    string read_line;

    while (stream >> read_line) {
        splitted_line.push_back(read_line);
    }
}

/*
 * Sprawdza, czy podany argument 'splitted_line':
 * 1) Ma rozmiar dwa lub trzy.
 * 2) 'splitted_line[0]' składa się z 3 do 11 znaków będących
 *    wielką literą alfabetu angielskiego lub cyfrą,
 *    przy czym pierwszy znak musi być literą.
 * 3) Pola opisujące czas ('splitted_line[1]', 'splitted_line[2]')
 *    składają się z godziny i minuty rozdzielonych kropką.
 *    Godzina może być wyrażona jedną lub dwoma cyframi.
 *    Minuta jest wyrażona zawsze dwoma cyframi.
 *    Poprawne wartości czasu zaczynają się od godziny 8.00 i
 *    kończą się o godzinie 20.00.
 * 4) Jeśli podane są dwie godziny, to oznaczają odpowiedni przedział czasowy.
 *
 * Jeśli wszystkie warunki są spełnione, daje 'true', w.p.p daje 'false'.
 */
bool is_data_correct(const vector<string> &splitted_line) {

    regex pattern_registration("^[A-Z][A-Z0-9]{2,10}$");
    regex pattern_time("^[0-9]{0,1}[0-9]\\.[0-9][0-9]$");
    int32_t parking_time;
    int32_t parking_start_time;
    int32_t parking_end_time;

    // -> Sprawdzenie liczby elementów.
    // -> Sprawdzenie poprawności formatu pierwszej godziny.
    if ((splitted_line.size() != 3 && splitted_line.size() != 2) ||
        !(regex_match(splitted_line[1], pattern_time))) {

        return false;
    }
    parking_start_time = time_to_minutes(splitted_line[1]);

    // -> Czy pierwsza godzina jest w przedziale 8.00 - 20.00.
    // -> Sprawdzenie poprawności numeru rejestracyjnego.
    if (parking_start_time < 0 ||
        !regex_match(splitted_line[0], pattern_registration)) {

        return false;
    }

    if (splitted_line.size() == 3) {

        // -> Sprawdzenie poprawności formatu drugiej godziny.
        if (!(regex_match(splitted_line[2], pattern_time))) {
            return false;
        }
        parking_end_time = time_to_minutes(splitted_line[2]);

        // -> Czy druga godzina jest w przedziale 8.00 - 20.00.
        // -> Czy ktoś kupił bilet od 8.00 do 20.00
        // (Jedyny możliwy przypadek za długiego czasu parkowania).
        if (parking_end_time < 0 ||
            (parking_start_time == 0 &&
            parking_end_time == MINUTES_IN_TWELVE_HOURS)) {

            return false;
        }

        parking_time = parking_start_time <= parking_end_time ?
        // Początek i koniec parkowania tego samego dnia.
        parking_end_time - parking_start_time :
        // Początek i koniec parkowania w różnych dniach.
        parking_end_time - parking_start_time + MINUTES_IN_TWELVE_HOURS;

        // -> Czy czas parkowania to co najmniej 10 minut.
        if (parking_time < TEN_MINUTES) {
            return false;
        }
    }

    return true;
}

/*
 * Na podstawie 'current_time'
 * zmienia oryginalne wartości 'day_number', 'last_known_time'.
 *
 * Celem jest aktualizacja czasu symulowanego w programie.
 */
void update_current_time(const string &current_time,
    int32_t &day_number, int32_t &last_known_time) {

    int32_t current_time_in_minutes = time_to_minutes(current_time);
    if (current_time_in_minutes < last_known_time) {
        day_number++;
    }
    last_known_time = current_time_in_minutes;
}

/*
 * Gdy dla danej rejestracji jest kupowany bilet, chcemy zmienić w bazie
 * czas końca parkowania tej rejestracji na większą wartość z:
 * 1) Wartości na kupowanym bilecie.
 * 2) Wartości, która już jest w bazie.
 *
 *  Ta funkcja znajduje i daje w wyniku ten późniejszy czas.
 */
int32_t choose_parking_end_time(const vector<string> &splitted_line,
    unordered_map<string, int32_t> &map, const int32_t day_number,
    int32_t parking_start_time, int32_t parking_end_time) {

    // Jeśli ktoś kupił bilet, który kończy się następnego dnia,
    // to do końca czasu biletu dodajemy jeden dzień.
    // Trzeba też sprawdzić, czy jakaś stara wartość czasu końca parkowania
    // faktycznie jest już w pamięci.
    return max(time_to_minutes(splitted_line[2], day_number +
        (parking_start_time > parking_end_time ? 1 : 0)),
        (map.count(splitted_line[0]) > 0 ? map[splitted_line[0]] : 0));
}

/*
 * Zakłada, że podane argumenty są poprawne (spełniają 'is_data_correct()').
 *
 * W zależności od argumentów:
 * 1) Zapisuje informacje o uiszczeniu opłaty oraz
 *    wypisuje na standardowe wyjście komunikat "OK" + 'line_number'.
 * 2) Sprawdza, czy dany numer rejestracyjny ma uiszczoną opłatę.
 *    Jeśli tak, to na standardowe wyjście wypisuje "YES" + 'line_number',
 *    jeśli nie, to na standardowe wyjście wypisuje "NO" + 'line_number'.
 */
void handle_request(const vector<string> &splitted_line,
    unordered_map<string, int32_t> &map,
    const int32_t day_number,
    const int32_t line_number,
    priority_queue<pair<string, int32_t>, vector<pair<string, int32_t>>,
        Queue_comparator> &earliest_expiring_ticket) {

    if (splitted_line.size() == 3) { // Kupowanie biletu.
        // Zapisanie biletu do kolejki i mapy.
        map[splitted_line[0]] = choose_parking_end_time(splitted_line, map,
            day_number, time_to_minutes(splitted_line[1]),
            time_to_minutes(splitted_line[2]));

        earliest_expiring_ticket.emplace(splitted_line[0],
            map[splitted_line[0]]);

        cout << "OK " << line_number << endl;
    }
    else { // Sprawdzenie ważności biletu.
        // Jeśli rejestracja jest w mapie i jej bilet jest ważny.
        if (map.count(splitted_line[0]) &&
            (time_to_minutes(splitted_line[1], day_number) <=
            map[splitted_line[0]])) {

            cout << "YES " << line_number << endl;
        }
        else {
            cout << "NO " << line_number << endl;
        }
    }
}

/*
 * Usuwa z pamięci nieaktualne opłaty za parkowanie i przypisane
 * do nich rejestracje.
 */
void remove_expired_tickets(unordered_map<string, int32_t> &map,
    priority_queue<pair<string, int32_t>, vector<pair<string, int32_t>>,
        Queue_comparator> &earliest_expiring_ticket,
    int32_t day_number,
    int32_t last_known_time) {

    // Jeśli aktualny czas jest większy niż najmniejszy czas w kolejce,
    // to jakiś bilet jest nieważny i go usuwamy.
    while (!earliest_expiring_ticket.empty() &&
        day_number * MINUTES_IN_A_DAY + last_known_time >
        earliest_expiring_ticket.top().second) {

        // Usuwamy niepotrzebny element z mapy.
        map.erase(earliest_expiring_ticket.top().first);

        // Usuwamy niepotrzebny element z kolejki.
        earliest_expiring_ticket.pop();
    }
}

/*
 * Symuluje parking opisany w zadaniu.
 * Wczytuje dane ze standardowego wejścia.
 * W przypadku błędnych danych wypisuje na standardowe wyjście diagnostyczne
 * napis "ERROR L", gdzie 'L' jest numerem wiersza, w którym są błędne dane.
 */
void parking() {
    string input;

    // Wczytany wiersz podzielony na rejestrację, godzinę, godzinę.
    vector<string> splitted_line;

    // Kluczem jest rejestracja, a wartością czas końca parkowania.
    unordered_map<string, int32_t> map;

    // Na początku kolejki jest ta rejestracja,
    // której czas parkowania kończy się najwcześniej.
    // Celem kolejki jest przyspieszenie znajdowania nieaktualnych biletów.
    priority_queue<pair<string, int32_t>, vector<pair<string, int32_t>>,
        Queue_comparator> earliest_expiring_ticket;

    int32_t line_number = 1;
    int32_t day_number = 0;

    // Tylko godzina i minuty, nie uwzględnia numeru aktualnego dnia.
    int32_t last_known_time = 0;

    while (getline(cin, input)) {
        split_line(splitted_line, input);
        if (is_data_correct(splitted_line)) {
            update_current_time(splitted_line[1],
                day_number, last_known_time);

            remove_expired_tickets(map, earliest_expiring_ticket,
                day_number, last_known_time);

            handle_request(splitted_line, map, day_number,
                line_number, earliest_expiring_ticket);
        }
        else {
            // Jeśli dane nie są poprawne, to
            // piszemy na standardowe wyjście diagnostyczne.
            cerr << "ERROR " << line_number << endl;
        }

        line_number++;
        splitted_line.clear();
    }
}

int main(void) {

    parking();

    return 0;
}
