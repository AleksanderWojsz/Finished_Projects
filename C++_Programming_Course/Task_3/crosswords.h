#ifndef CROSSWORDS_H
#define CROSSWORDS_H

#include <list>
#include <iostream>
#include <string>
#include <unordered_map>

constexpr char DEFAULT_CHAR('?');
constexpr std::string_view DEFAULT_WORD= "?"; // Nie char*, bo w przykładzie jest 'DEFAULT_WORD.length()'.
extern char CROSSWORD_BACKGROUND;

using dim_t = std::pair<size_t, size_t>;
using pos_t = std::pair<size_t, size_t>;

enum orientation_t : bool {
    V = false,
    H = true
};

class RectArea {
private:
    pos_t left_top;
    pos_t right_bottom;

public:
    RectArea(const RectArea &other);
    RectArea(RectArea &&other) noexcept;
    RectArea& operator=(const RectArea &other);
    RectArea& operator=(RectArea &&other) noexcept;
    pos_t get_left_top() const;
    pos_t get_right_bottom() const;
    void set_left_top(const pos_t &new_left_top);
    void set_right_bottom(const pos_t &new_right_bottom);
    RectArea operator*(const RectArea &other) const;
    RectArea &operator*=(const RectArea &other);
    dim_t size() const;
    bool empty() const;
    void embrace(const pos_t &point);

    constexpr RectArea(pos_t left_top, pos_t right_bottom)
            : left_top(left_top), right_bottom(right_bottom) { }
};

// Coordinates, so that the size of the rectangle is 0x0.
constexpr RectArea DEFAULT_EMPTY_RECT_AREA({1, 1}, {0, 0});

class Word {
private:
    pos_t start_position;
    orientation_t orientation;
    std::string value;
    RectArea area_where_the_word_is;

public:
    Word(size_t x, size_t y, orientation_t orientation, const std::string& value);
    Word(const Word &other);
    Word(Word &&other) noexcept ;
    Word& operator=(const Word &other);
    Word& operator=(Word &&other) noexcept ;
    pos_t get_start_position() const;
    pos_t get_end_position() const;
    orientation_t get_orientation() const;
    char at(size_t index) const;
    size_t length() const;
    bool operator==(const Word& other) const;
    bool operator!=(const Word& other) const;
    RectArea rect_area() const;


    // Called whenever values are compared using <, >, <=, >=, or <=>.
    // Defined in the header file because auto return type must be defined before it's used.
    auto operator<=>(const Word &other) const {

        auto cmp_1 = start_position.first <=> other.start_position.first;
        if (cmp_1 != 0) { // If they are equal, we compare other parameters.
            return cmp_1;
        }

        auto cmp_2 = start_position.second <=> other.start_position.second;
        if (cmp_2 != 0) {
            return cmp_2;
        }

        // We assume that V > H.
        return other.orientation <=> orientation;
    }
};

class Crossword {
private:
    std::list<Word> words;
    RectArea area;
    std::unordered_map<size_t, std::unordered_map<size_t, char>> board;
    dim_t wordcounter;
public:
    Crossword(const Word& first_word, std::initializer_list<Word> next_words);
    Crossword(const Crossword &other);
    Crossword(Crossword &&other) noexcept;
    Crossword& operator= (const Crossword &other);
    Crossword& operator= (Crossword &&other) noexcept;
    bool insert_word(const Word &word);
    dim_t size();
    dim_t word_count();
    Crossword operator+(const Crossword &other);
    Crossword& operator+=(const Crossword &other);
    friend std::ostream &operator<<(std::ostream &os, const Crossword &crossword);
private:
    bool check_collisions(const Word &checked);
};

#endif // CROSSWORDS_H
