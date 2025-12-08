#pragma once

#include "card.hpp"
#include <vector>
#include <cstddef>

class Deck {
public:
    Deck();

    void shuffle();
    Card deal();
    std::size_t size() const;

private:
    std::vector<Card> cards_;
    std::size_t next_card_;
};
