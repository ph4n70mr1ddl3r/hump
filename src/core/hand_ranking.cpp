#include "hand_ranking.hpp"
#include <algorithm>
#include <map>
#include <stdexcept>

// Forward declarations


struct CardValue {
    int rank; // 0-12
    int suit; // 0-3
};

std::vector<int> getComparisonKey(const std::vector<CardValue>& cards, HandRank rank);

std::vector<CardValue> toCardValues(const std::vector<Card>& cards) {
    std::vector<CardValue> values;
    values.reserve(cards.size());
    for (const auto& card : cards) {
        values.push_back({static_cast<int>(card.rank()), static_cast<int>(card.suit())});
    }
    return values;
}



bool isStraight(const std::vector<int>& ranks) {
    // Check for straight (Ace can be low for A-2-3-4-5)
    if (ranks.size() < 5) return false;
    
    // Check normal straight
    for (size_t i = 0; i <= ranks.size() - 5; ++i) {
        bool straight = true;
        for (size_t j = 0; j < 4; ++j) {
            if (ranks[i + j] != ranks[i + j + 1] + 1) {
                straight = false;
                break;
            }
        }
        if (straight) return true;
    }
    
    // Check Ace-low straight (A, 2, 3, 4, 5)
    if (ranks[0] == 12) { // Ace
        std::vector<int> low_ranks = {12, 0, 1, 2, 3};
        bool has_all = true;
        for (int r : low_ranks) {
            if (std::find(ranks.begin(), ranks.end(), r) == ranks.end()) {
                has_all = false;
                break;
            }
        }
        if (has_all) return true;
    }
    
    return false;
}

bool isFlush(const std::vector<CardValue>& cards) {
    if (cards.size() < 5) return false;
    
    std::map<int, int> suit_count;
    for (const auto& card : cards) {
        suit_count[card.suit]++;
    }
    
    for (const auto& [suit, count] : suit_count) {
        if (count >= 5) return true;
    }
    return false;
}

HandRank evaluateFiveCards(const std::vector<CardValue>& cards) {
    if (cards.size() != 5) {
        throw std::invalid_argument("Need exactly 5 cards for evaluation");
    }
    
    std::vector<int> ranks;
    for (const auto& card : cards) {
        ranks.push_back(card.rank);
    }
    std::sort(ranks.rbegin(), ranks.rend()); // Descending
    
    // Count rank frequencies
    std::map<int, int> rank_count;
    for (int rank : ranks) {
        rank_count[rank]++;
    }
    
    bool is_flush = isFlush(cards);
    bool is_straight = isStraight(ranks);
    
    // Check for straight flush / royal flush
    if (is_flush && is_straight) {
        // Check for Ace-high straight (royal flush)
        if (ranks[0] == 12 && ranks[4] == 8) { // A, K, Q, J, T
            return HandRank::ROYAL_FLUSH;
        }
        return HandRank::STRAIGHT_FLUSH;
    }
    
    // Check four of a kind
    for (const auto& [rank, count] : rank_count) {
        if (count == 4) return HandRank::FOUR_OF_A_KIND;
    }
    
    // Check full house
    bool has_three = false;
    bool has_pair = false;
    for (const auto& [rank, count] : rank_count) {
        if (count == 3) has_three = true;
        if (count == 2) has_pair = true;
    }
    if (has_three && has_pair) return HandRank::FULL_HOUSE;
    
    if (is_flush) return HandRank::FLUSH;
    if (is_straight) return HandRank::STRAIGHT;
    if (has_three) return HandRank::THREE_OF_A_KIND;
    
    // Check two pair
    int pair_count = 0;
    for (const auto& [rank, count] : rank_count) {
        if (count == 2) pair_count++;
    }
    if (pair_count == 2) return HandRank::TWO_PAIR;
    if (pair_count == 1) return HandRank::ONE_PAIR;
    
    return HandRank::HIGH_CARD;
}

// Helper to generate all 5-card combinations from cards (size 6 or 7)
std::vector<std::vector<CardValue>> generateCombinations(const std::vector<CardValue>& cards) {
    std::vector<std::vector<CardValue>> combos;
    int n = cards.size();
    int k = 5;
    
    // Generate indices
    std::vector<int> indices(k);
    for (int i = 0; i < k; ++i) indices[i] = i;
    
    while (true) {
        // Add current combination
        std::vector<CardValue> combo;
        combo.reserve(k);
        for (int i : indices) {
            combo.push_back(cards[i]);
        }
        combos.push_back(std::move(combo));
        
        // Next combination
        int i = k - 1;
        while (i >= 0 && indices[i] == n - k + i) {
            --i;
        }
        if (i < 0) break;
        ++indices[i];
        for (int j = i + 1; j < k; ++j) {
            indices[j] = indices[j - 1] + 1;
        }
    }
    
    return combos;
}

// Evaluate best hand from 6 or 7 cards
HandRank evaluateBestFiveCards(const std::vector<CardValue>& cards, std::vector<CardValue>& best_combo) {
    if (cards.size() < 5 || cards.size() > 7) {
        throw std::invalid_argument("Need 5-7 cards for evaluation");
    }
    
    if (cards.size() == 5) {
        best_combo = cards;
        return evaluateFiveCards(cards);
    }
    
    auto combos = generateCombinations(cards);
    HandRank best_rank = HandRank::HIGH_CARD;
    size_t best_index = 0;
    
    for (size_t i = 0; i < combos.size(); ++i) {
        HandRank rank = evaluateFiveCards(combos[i]);
        if (rank > best_rank) {
            best_rank = rank;
            best_index = i;
        } else if (rank == best_rank) {
            // Same rank, compare kickers
            std::vector<int> key_best = getComparisonKey(combos[best_index], rank);
            std::vector<int> key_current = getComparisonKey(combos[i], rank);
            // Lexicographic comparison (higher key is better)
            for (size_t j = 0; j < std::min(key_best.size(), key_current.size()); ++j) {
                if (key_current[j] != key_best[j]) {
                    if (key_current[j] > key_best[j]) {
                        best_index = i; // current combo is better
                    }
                    break;
                }
            }
            // If keys are equal, keep first found (no change)
        }
    }
    
    best_combo = std::move(combos[best_index]);
    return best_rank;
}



std::vector<int> getComparisonKey(const std::vector<CardValue>& cards, HandRank rank) {
    std::vector<int> ranks;
    for (const auto& card : cards) {
        ranks.push_back(card.rank);
    }
    std::sort(ranks.rbegin(), ranks.rend());
    
    std::map<int, int> rank_count;
    for (int rank_val : ranks) {
        rank_count[rank_val]++;
    }
    
    std::vector<int> key;
    
    switch (rank) {
        case HandRank::HIGH_CARD:
            // Just sorted ranks
            key = ranks;
            break;
            
        case HandRank::ONE_PAIR: {
            int pair_rank = -1;
            std::vector<int> kickers;
            for (const auto& [r, count] : rank_count) {
                if (count == 2) pair_rank = r;
                else kickers.push_back(r);
            }
            std::sort(kickers.rbegin(), kickers.rend());
            key.push_back(pair_rank);
            key.insert(key.end(), kickers.begin(), kickers.end());
            break;
        }
            
        case HandRank::TWO_PAIR: {
            std::vector<int> pairs;
            int kicker = -1;
            for (const auto& [r, count] : rank_count) {
                if (count == 2) pairs.push_back(r);
                else kicker = r;
            }
            std::sort(pairs.rbegin(), pairs.rend());
            key.push_back(pairs[0]);
            key.push_back(pairs[1]);
            key.push_back(kicker);
            break;
        }
            
        case HandRank::THREE_OF_A_KIND: {
            int trips_rank = -1;
            std::vector<int> kickers;
            for (const auto& [r, count] : rank_count) {
                if (count == 3) trips_rank = r;
                else kickers.push_back(r);
            }
            std::sort(kickers.rbegin(), kickers.rend());
            key.push_back(trips_rank);
            key.insert(key.end(), kickers.begin(), kickers.end());
            break;
        }
            
        case HandRank::STRAIGHT:
        case HandRank::FLUSH:
        case HandRank::STRAIGHT_FLUSH:
        case HandRank::ROYAL_FLUSH: {
            // For straight, need to handle Ace-low straight (5-4-3-2-A)
            // For Ace-low straight, the highest card is 5 (rank 3)
            bool is_ace_low_straight = false;
            if (rank == HandRank::STRAIGHT || rank == HandRank::STRAIGHT_FLUSH || rank == HandRank::ROYAL_FLUSH) {
                // Check for Ace-low straight: A,2,3,4,5
                std::vector<int> straight_ranks = ranks;
                std::sort(straight_ranks.begin(), straight_ranks.end()); // ascending
                if (straight_ranks[0] == 0 && straight_ranks[1] == 1 && straight_ranks[2] == 2 &&
                    straight_ranks[3] == 3 && straight_ranks[4] == 12) {
                    is_ace_low_straight = true;
                }
            }
            if (is_ace_low_straight) {
                // For Ace-low straight, the effective highest card is 5 (rank 3)
                key.push_back(3); // 5 is rank 3
            } else {
                // Normal straight: highest card
                key.push_back(ranks[0]);
            }
            break;
        }
            
        case HandRank::FULL_HOUSE: {
            int trips_rank = -1;
            int pair_rank = -1;
            for (const auto& [r, count] : rank_count) {
                if (count == 3) trips_rank = r;
                else if (count == 2) pair_rank = r;
            }
            key.push_back(trips_rank);
            key.push_back(pair_rank);
            break;
        }
            
        case HandRank::FOUR_OF_A_KIND: {
            int quad_rank = -1;
            int kicker = -1;
            for (const auto& [r, count] : rank_count) {
                if (count == 4) quad_rank = r;
                else kicker = r;
            }
            key.push_back(quad_rank);
            key.push_back(kicker);
            break;
        }
    }
    
    return key;
}

HandRank HandRanking::evaluate(const std::vector<Card>& cards) {
    if (cards.size() < 5 || cards.size() > 7) {
        throw std::invalid_argument("Hand evaluation requires 5-7 cards");
    }
    
    auto card_values = toCardValues(cards);
    
    // For 5 cards, evaluate directly
    if (cards.size() == 5) {
        return evaluateFiveCards(card_values);
    }
    
    // For 6 or 7 cards, evaluate all 5-card combinations and pick best
    int n = card_values.size();
    int k = 5;
    HandRank best_rank = HandRank::HIGH_CARD;
    
    // Generate all combinations of indices
    std::vector<int> indices(k);
    for (int i = 0; i < k; ++i) indices[i] = i;
    
    while (true) {
        // Build current combination
        std::vector<CardValue> combo;
        combo.reserve(k);
        for (int i : indices) {
            combo.push_back(card_values[i]);
        }
        
        HandRank rank = evaluateFiveCards(combo);
        if (rank > best_rank) {
            best_rank = rank;
        }
        // Note: for ties (same rank), we'd need to compare kickers
        // but for evaluation alone, any hand with same rank is equivalent
        
        // Next combination
        int i = k - 1;
        while (i >= 0 && indices[i] == n - k + i) {
            --i;
        }
        if (i < 0) break;
        ++indices[i];
        for (int j = i + 1; j < k; ++j) {
            indices[j] = indices[j - 1] + 1;
        }
    }
    
    return best_rank;
}

std::string HandRanking::rankToString(HandRank rank) {
    switch (rank) {
        case HandRank::HIGH_CARD: return "HIGH_CARD";
        case HandRank::ONE_PAIR: return "ONE_PAIR";
        case HandRank::TWO_PAIR: return "TWO_PAIR";
        case HandRank::THREE_OF_A_KIND: return "THREE_OF_A_KIND";
        case HandRank::STRAIGHT: return "STRAIGHT";
        case HandRank::FLUSH: return "FLUSH";
        case HandRank::FULL_HOUSE: return "FULL_HOUSE";
        case HandRank::FOUR_OF_A_KIND: return "FOUR_OF_A_KIND";
        case HandRank::STRAIGHT_FLUSH: return "STRAIGHT_FLUSH";
        case HandRank::ROYAL_FLUSH: return "ROYAL_FLUSH";
        default: return "UNKNOWN";
    }
}

int HandRanking::compare(const std::vector<Card>& hand1, const std::vector<Card>& hand2) {
    auto cards1 = toCardValues(hand1);
    auto cards2 = toCardValues(hand2);
    
    std::vector<CardValue> best_combo1, best_combo2;
    HandRank rank1 = evaluateBestFiveCards(cards1, best_combo1);
    HandRank rank2 = evaluateBestFiveCards(cards2, best_combo2);
    
    if (rank1 != rank2) {
        return static_cast<int>(rank1) - static_cast<int>(rank2);
    }
    
    std::vector<int> key1 = getComparisonKey(best_combo1, rank1);
    std::vector<int> key2 = getComparisonKey(best_combo2, rank2);
    
    // Lexicographic comparison
    for (size_t i = 0; i < std::min(key1.size(), key2.size()); ++i) {
        if (key1[i] != key2[i]) {
            return key1[i] - key2[i];
        }
    }
    
    // All compared elements equal
    return 0;
}