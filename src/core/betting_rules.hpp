#pragma once

#include <cstdint>
#include <vector>
#include <string>

enum class Action {
    FOLD,
    CALL,
    RAISE
};

struct BettingRound {
    int current_bet; // amount to call
    int min_raise;   // minimum raise amount
    int pot;         // current pot size
};

class BettingRules {
public:
    // Validate if an action is valid given current betting round and player stack
    static bool isValidAction(Action action, int amount, const BettingRound& round, int player_stack);

    // Calculate minimum raise amount given current bet
    static int calculateMinRaise(int current_bet, int big_blind);

    // Calculate maximum raise amount (player's stack)
    static int calculateMaxRaise(int player_stack);

    // Apply action to betting round and return updated round
    static BettingRound applyAction(Action action, int amount, const BettingRound& round, int player_stack);

    // Check if betting round is complete (all players have called or folded)
    static bool isRoundComplete(const std::vector<bool>& has_acted, const std::vector<int>& bets, int current_bet);
};
