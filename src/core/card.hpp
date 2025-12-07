#pragma once

#include <string>
#include <cstdint>

enum class Rank {
    TWO = 0,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    ACE
};

enum class Suit {
    CLUBS = 0,
    DIAMONDS,
    HEARTS,
    SPADES
};

class Card {
public:
    Card() = default;
    Card(const std::string& str);
    Card(Rank rank, Suit suit);
    
    Rank rank() const;
    Suit suit() const;
    std::string toString() const;
    uint8_t toInt() const;
    
    bool operator==(const Card& other) const;
    bool operator!=(const Card& other) const;
    
private:
    uint8_t value_; // 0-51: rank * 4 + suit
};