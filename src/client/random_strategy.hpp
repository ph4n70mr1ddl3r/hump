#pragma once

#include <string>
#include <vector>
#include <random>

class RandomStrategy {
public:
    RandomStrategy();

    // Choose an action given possible actions and betting context
    // Returns a pair of (action, amount). For fold/call, amount is 0.
    // For raise, amount is between min_raise and max_raise (inclusive).
    std::pair<std::string, int> chooseAction(
        const std::vector<std::string>& possible_actions,
        int call_amount,
        int min_raise,
        int max_raise);

private:
    std::mt19937 rng_;
};
