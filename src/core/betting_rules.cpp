#include "betting_rules.hpp"
#include "../common/constants.hpp"
#include <algorithm>

bool BettingRules::isValidAction(Action action, int amount, const BettingState& round, int player_stack) {
    switch (action) {
        case Action::FOLD:
            return amount == 0;
        case Action::CALL:
            return amount == round.current_bet && amount <= player_stack;
        case Action::RAISE:
            return amount >= round.min_raise && amount <= player_stack && amount > round.current_bet;
        default:
            return false;
    }
}

int BettingRules::calculateMinRaise(int current_bet, int big_blind) {
    if (current_bet == 0) {
        return big_blind;
    }
    return current_bet + big_blind;
}

int BettingRules::calculateMaxRaise(int player_stack) {
    return player_stack;
}

BettingState BettingRules::applyAction(Action action, int amount, const BettingState& round, int player_stack) {
    BettingState new_round = round;
    switch (action) {
        case Action::FOLD:
            // No change to pot or bets
            break;
        case Action::CALL:
            new_round.pot += amount;
            break;
        case Action::RAISE:
            new_round.pot += amount;
            new_round.current_bet = amount;
            new_round.min_raise = calculateMinRaise(amount, common::constants::BIG_BLIND);
            break;
    }
    return new_round;
}

bool BettingRules::isRoundComplete(const std::vector<bool>& has_acted, const std::vector<int>& bets, int current_bet) {
    if (has_acted.size() != bets.size()) return false;
    bool all_acted = true;
    for (bool acted : has_acted) {
        if (!acted) {
            all_acted = false;
            break;
        }
    }
    if (!all_acted) return false;
    // Check if all bets are equal to current bet (or FOLDED_BET if player folded)
    for (int bet : bets) {
        if (bet != current_bet && bet != FOLDED_BET) {
            return false;
        }
    }
    return true;
}