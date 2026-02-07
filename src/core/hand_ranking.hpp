#pragma once

#include "card.hpp"
#include <vector>
#include <string>

enum class HandRank {
    HIGH_CARD,
    ONE_PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH
};

class HandRanking {
public:
    static HandRank evaluate(const std::vector<Card>& cards);
    static std::string rankToString(HandRank rank) noexcept;

    // For comparing two hands of same rank
    static int compare(const std::vector<Card>& hand1, const std::vector<Card>& hand2);
};
