#include "deck.hpp"
#include <algorithm>
#include <random>
#include <stdexcept>

namespace {

std::vector<Card> createStandardDeck() {
    std::vector<Card> deck;
    deck.reserve(52);

    for (int rank = 0; rank < 13; ++rank) {
        for (int suit = 0; suit < 4; ++suit) {
            deck.emplace_back(static_cast<Rank>(rank), static_cast<Suit>(suit));
        }
    }

    return deck;
}

} // anonymous namespace

Deck::Deck() : cards_(createStandardDeck()), next_card_(0) {
    shuffle();
}

void Deck::shuffle() {
    static std::random_device rd;
    static std::mt19937 g(rd());

    std::shuffle(cards_.begin(), cards_.end(), g);
    next_card_ = 0;
}

Card Deck::deal() {
    if (next_card_ >= cards_.size()) {
        throw std::out_of_range("No cards left in deck");
    }

    return cards_[next_card_++];
}

std::size_t Deck::size() const {
    return cards_.size() - next_card_;
}