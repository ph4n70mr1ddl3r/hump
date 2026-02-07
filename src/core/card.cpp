#include "card.hpp"
#include <stdexcept>
#include <cctype>

namespace {

constexpr std::size_t NUM_RANKS = 13;
constexpr std::size_t NUM_SUITS = 4;

constexpr char rankToChar(Rank rank) noexcept {
    switch (rank) {
        case Rank::TWO: return '2';
        case Rank::THREE: return '3';
        case Rank::FOUR: return '4';
        case Rank::FIVE: return '5';
        case Rank::SIX: return '6';
        case Rank::SEVEN: return '7';
        case Rank::EIGHT: return '8';
        case Rank::NINE: return '9';
        case Rank::TEN: return 'T';
        case Rank::JACK: return 'J';
        case Rank::QUEEN: return 'Q';
        case Rank::KING: return 'K';
        case Rank::ACE: return 'A';
        default: return '?';
    }
}

Rank charToRank(char c) {
    switch (c) {
        case '2': return Rank::TWO;
        case '3': return Rank::THREE;
        case '4': return Rank::FOUR;
        case '5': return Rank::FIVE;
        case '6': return Rank::SIX;
        case '7': return Rank::SEVEN;
        case '8': return Rank::EIGHT;
        case '9': return Rank::NINE;
        case 'T': return Rank::TEN;
        case 'J': return Rank::JACK;
        case 'Q': return Rank::QUEEN;
        case 'K': return Rank::KING;
        case 'A': return Rank::ACE;
        default: throw std::invalid_argument("Invalid rank character");
    }
}

constexpr char suitToChar(Suit suit) noexcept {
    switch (suit) {
        case Suit::CLUBS: return 'c';
        case Suit::DIAMONDS: return 'd';
        case Suit::HEARTS: return 'h';
        case Suit::SPADES: return 's';
        default: return '?';
    }
}

Suit charToSuit(char c) {
    switch (c) {
        case 'c': return Suit::CLUBS;
        case 'd': return Suit::DIAMONDS;
        case 'h': return Suit::HEARTS;
        case 's': return Suit::SPADES;
        default: throw std::invalid_argument("Invalid suit character");
    }
}

} // anonymous namespace

Card::Card(const std::string& str) {
    if (str.length() != 2) {
        throw std::invalid_argument("Card string must be 2 characters");
    }

    if (!std::isprint(str[0]) || !std::isprint(str[1])) {
        throw std::invalid_argument("Card string contains non-printable characters");
    }

    Rank rank = charToRank(str[0]);
    Suit suit = charToSuit(str[1]);
    value_ = static_cast<uint8_t>(rank) * NUM_SUITS + static_cast<uint8_t>(suit);
}

Card::Card(Rank rank, Suit suit) {
    value_ = static_cast<uint8_t>(rank) * NUM_SUITS + static_cast<uint8_t>(suit);
}

Rank Card::rank() const {
    return static_cast<Rank>(value_ / NUM_SUITS);
}

Suit Card::suit() const {
    return static_cast<Suit>(value_ % NUM_SUITS);
}

std::string Card::toString() const {
    std::string result;
    result += rankToChar(rank());
    result += suitToChar(suit());
    return result;
}

uint8_t Card::toInt() const {
    return value_;
}

bool Card::operator==(const Card& other) const {
    return value_ == other.value_;
}

bool Card::operator!=(const Card& other) const {
    return !(*this == other);
}