#include "crosswords.h"
#include <string>
#include <utility>
#include <cassert>

constexpr size_t MAX_SIZE_T = (size_t) - 1;
constinit char CROSSWORD_BACKGROUND('.');

// --- Word implementation --- //

// Trims the word if coordinates would exceed 'MAX_SIZE_T', or changes to 'DEFAULT_WORD' if the value is empty.
static std::string modify_value_if_needed(size_t x, size_t y, orientation_t orientation, const std::string& value) {

    if (value.empty()) {
        return std::string(DEFAULT_WORD);
    }
    else if (orientation == H && x > MAX_SIZE_T - value.length() + 1) { // Overflow won't occur as value.length() >= 1.
        return value.substr(0, MAX_SIZE_T - x + 1);
    }
    else if (orientation == V && y > MAX_SIZE_T - value.length() + 1) {
        return value.substr(0, MAX_SIZE_T - y + 1);
    }
    else {
        return value;
    }
}

Word::Word(size_t x, size_t y, orientation_t orientation, const std::string& value)
        : start_position(x, y), orientation(orientation),
          value(modify_value_if_needed(x, y, orientation, value)),
          area_where_the_word_is({x, y},
          {(orientation == H ? x + modify_value_if_needed(x, y, orientation, value).length() - 1: x),
          (orientation == V ? y + modify_value_if_needed(x, y, orientation, value).length() - 1: y)}) {
}

// Move constructor
Word::Word(Word &&other) noexcept
        : start_position(std::move(other.start_position)), orientation(other.orientation),
          value(std::move(other.value)), area_where_the_word_is(std::move(other.area_where_the_word_is)) {

    other.start_position = {0, 0};
    other.area_where_the_word_is = DEFAULT_EMPTY_RECT_AREA;
}

// Copy constructor
Word::Word(const Word &other)
        : start_position(other.start_position), orientation(other.orientation),
          value(other.value), area_where_the_word_is(other.area_where_the_word_is) {
}

// Move assignment operator
Word &Word::operator=(Word &&other) noexcept {

    if (this != &other) {
        start_position = std::move(other.start_position);
        orientation = other.orientation;
        value = std::move(other.value);
        area_where_the_word_is = std::move(other.area_where_the_word_is);

        other.start_position = {0, 0};
        other.area_where_the_word_is = DEFAULT_EMPTY_RECT_AREA;
    }

    return *this;
}

// Copy assignment operator
Word &Word::operator=(const Word &other) {

    if (this != &other) {
        start_position = other.start_position;
        orientation = other.orientation;
        value = other.value;
        area_where_the_word_is = other.area_where_the_word_is;
    }

    return *this;
}

pos_t Word::get_start_position() const {
    return start_position;
}

pos_t Word::get_end_position() const {
    if (orientation == H) {
        return pos_t(start_position.first + value.length() - 1, start_position.second);
    } else {
        return pos_t(start_position.first, start_position.second + value.length() - 1);
    }
}

orientation_t Word::get_orientation() const {
    return orientation;
}

char Word::at(size_t index) const {

    return (index >= value.length() ? '?' : value.at(index));
}

size_t Word::length() const {
    return value.length();
}

bool Word::operator==(const Word& other) const {
    return start_position.first == other.start_position.first &&
           start_position.second == other.start_position.second &&
           orientation == other.orientation;
}

bool Word::operator!=(const Word &other) const {
    return !(*this == other);
}

RectArea Word::rect_area() const {
    return area_where_the_word_is;
}

// --- RectArea implementation --- //

// Copy constructor.
RectArea::RectArea(const RectArea &other)
        : left_top(other.left_top), right_bottom(other.right_bottom) {}

// Move constructor.
RectArea::RectArea(RectArea &&other) noexcept
        : left_top(std::move(other.left_top)), right_bottom(std::move(other.right_bottom)){
    other.left_top = {0, 0};
    other.right_bottom = {0, 0};
}

// Copy assignment operator
RectArea& RectArea::operator=(const RectArea &other) {
    if (this != &other) {
        left_top = other.left_top;
        right_bottom = other.right_bottom;
    }
    return *this;
}

// Move assignment operator
RectArea& RectArea::operator=(RectArea &&other) noexcept {
    if (this != &other) {
        left_top = std::move(other.left_top);
        right_bottom = std::move(other.right_bottom);
        other.left_top = {0, 0};
        other.right_bottom = {0, 0};
    }
    return *this;
}

pos_t RectArea::get_left_top() const {
    return left_top;
}

pos_t RectArea::get_right_bottom() const {
    return right_bottom;
}

void RectArea::set_left_top(const pos_t &new_left_top) {
    left_top = new_left_top;
}

void RectArea::set_right_bottom(const pos_t &new_right_bottom) {
    right_bottom = new_right_bottom;
}

RectArea RectArea::operator*(const RectArea &other) const {
    return RectArea({std::max(left_top.first, other.left_top.first),
                     std::max(left_top.second, other.left_top.second)},
                    {std::min(right_bottom.first, other.right_bottom.first),
                     std::min(right_bottom.second, other.right_bottom.second)});
}

RectArea &RectArea::operator*=(const RectArea &other) {
    return *this = *this * other;
}

bool RectArea::empty() const {
    return right_bottom.first < left_top.first || right_bottom.second < left_top.second;
}

dim_t RectArea::size() const {

    if (empty()) {
        return {0, 0};
    }
    else {
        assert((left_top.first != 0 || right_bottom.first != MAX_SIZE_T) &&
               (left_top.second != 0 || right_bottom.second != MAX_SIZE_T) &&
               "Size is too large; overflow would occur.");

        return {right_bottom.first - left_top.first + 1, right_bottom.second - left_top.second + 1};
    }
}

void RectArea::embrace(const pos_t &point) {

    if (empty()) { // If the area is empty, we set it to a point.
        left_top = point;
        right_bottom = point;
    }
    else {
        right_bottom.first = std::max(right_bottom.first, point.first);
        right_bottom.second = std::max(right_bottom.second, point.second);
        left_top.first = std::min(left_top.first, point.first);
        left_top.second = std::min(left_top.second, point.second);
    }
}


// --- Crossword implementation --- //

static void insert_to_board(const Word &word,
                            std::unordered_map<size_t, std::unordered_map<size_t, char>> &board) {
    pos_t where = word.get_start_position();
    size_t length = word.length();
    orientation_t orient = word.get_orientation();
    for (size_t i = 0; i < length; i++){
        board[where.second].insert({where.first, word.at(i)});
        if (orient == H)
            where.first++;
        else
            where.second++;
    }
}

Crossword::Crossword(const Word& first_word, std::initializer_list<Word> next_words)
        : words({first_word}), area(first_word.rect_area()), board(), wordcounter(0, 0) {
    if (first_word.get_orientation() == H)
        wordcounter.first++;
    else // word is orientated vertically
        wordcounter.second++;
    insert_to_board(first_word, board);
    for (const Word& w : next_words)
        insert_word(w);
}

Crossword::Crossword(const Crossword &other)
        : words(other.words), area(other.area), board(other.board), wordcounter(other.wordcounter) {
}

Crossword::Crossword(Crossword &&other) noexcept
        : words(std::move(other.words)), area(std::move(other.area)),
          board(std::move(other.board)), wordcounter(std::move(other.wordcounter)){
    other.words.clear();
    other.area = DEFAULT_EMPTY_RECT_AREA;
    other.board.clear();
    other.wordcounter = {0, 0};
}

Crossword& Crossword::operator=(const Crossword &other) {
    if (this != &other) {
        words = other.words;
        area = other.area;
        board = other.board;
        wordcounter = other.wordcounter;
    }
    return *this;
}

Crossword& Crossword::operator=(Crossword &&other) noexcept {
    if (this != &other) {
        words = std::move(other.words);
        area = std::move(other.area);
        board = std::move(other.board);
        wordcounter = std::move(other.wordcounter);
        other.words.clear();
        other.area = DEFAULT_EMPTY_RECT_AREA;
        other.board.clear();
        other.wordcounter = {0, 0};
    }
    return *this;
}

bool Crossword::insert_word(const Word &word) {
    if (!check_collisions(word)) {
        words.push_back(word);
        RectArea wordArea = word.rect_area();
        area.embrace(wordArea.get_left_top());
        area.embrace(wordArea.get_right_bottom());
        if (word.get_orientation() == H)
            wordcounter.first++;
        else // word is orientated vertically
            wordcounter.second++;
        insert_to_board(word, board);
        return true;
    }
    return false;
}

dim_t Crossword::size() {
    return area.size();
}

dim_t Crossword::word_count() {
    return wordcounter;
}

Crossword Crossword::operator+(const Crossword &other){
    return Crossword(*this) += other;
}

Crossword &Crossword::operator+=(const Crossword &other){
    if (this != &other)
        for (const Word& w : other.words)
            this->insert_word(w);
    return *this;
}

static char printableChar(const char &character){
    static constexpr char DIFF = 'A' - 'a';
    if (character >= 'A' && character <= 'Z')
        return character;
    else if (character >= 'a' && character <= 'z')
        return character + DIFF;
    else
        return DEFAULT_CHAR;
}

std::ostream& operator<<(std::ostream &os, const Crossword &crossword) {
    pos_t topleft = crossword.area.get_left_top();
    pos_t botright = crossword.area.get_right_bottom();
    dim_t area_size = crossword.area.size();
    for (size_t x = 0; x <= area_size.first; x++)
        os << CROSSWORD_BACKGROUND << ' ';
    os << CROSSWORD_BACKGROUND << '\n';
    for (size_t y = topleft.second; y <= botright.second; y++) {
        os << CROSSWORD_BACKGROUND << ' ';
        for (size_t x = topleft.first; x <= botright.first; x++) {
            if (crossword.board.contains(y) && crossword.board.at(y).contains(x))
                os << printableChar(crossword.board.at(y).at(x));
            else
                os << CROSSWORD_BACKGROUND;
            os << ' ';
            // check size_t overflow
            if (x + 1 == 0)
                break;
        }
        os << CROSSWORD_BACKGROUND << '\n';
        // check size_t overflow
        if (y + 1 == 0)
            break;
    }
    for (size_t x = 0; x <= area_size.first; x++)
        os << CROSSWORD_BACKGROUND << ' ';
    os << CROSSWORD_BACKGROUND << '\n';
    return os;
}

// creates area containing word + a space before and after it vertically
static RectArea get_buffer_area(const Word &word){
    pos_t checked_start = word.get_start_position();
    pos_t checked_end = word.get_end_position();
    pos_t buffer_start = {(checked_start.first != 0 ? checked_start.first - 1 : 0),
                            (checked_start.second != 0 ? checked_start.second - 1 : 0)};
    pos_t buffer_end = {(checked_end.first + 1 != 0 ? checked_end.first + 1 : checked_end.first),
                            (checked_end.second + 1 != 0 ? checked_end.second + 1 : checked_end.second)};
    return RectArea(buffer_start, buffer_end);
}

static char getChar(const Word &word, const pos_t &point) {
    pos_t beginning = word.get_start_position();
    return word.at(point.first - beginning.first + point.second - beginning.second);
}

bool Crossword::check_collisions(const Word &checked) {
    RectArea checked_area = checked.rect_area(),
            checked_buffer_area = get_buffer_area(checked);
    for (const Word& word : words) {
        RectArea word_area = word.rect_area();
        RectArea cross_area = checked_area * word_area;
        if (!cross_area.empty()) {
            pos_t cross_point = cross_area.get_left_top();
            // assert there is only one point in the cross_area
            // and if so - that both words have the same character on it
            // and that they are ortogonal
            if (cross_point != cross_area.get_right_bottom() ||
                printableChar(getChar(word, cross_point)) !=
                printableChar(getChar(checked, cross_point)) ||
                word.get_orientation() == checked.get_orientation())
                return true;
        }
        else if (!(checked_buffer_area * word_area).empty()) {
            // added word would interfere with buffer zone around
            // a word already existing in the crossword
            return true;
        }
    }
    return false;
}