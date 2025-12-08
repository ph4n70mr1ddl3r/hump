#include "random_strategy.hpp"
#include <algorithm>
#include <stdexcept>

RandomStrategy::RandomStrategy() : rng_(std::random_device{}()) {}

std::pair<std::string, int> RandomStrategy::chooseAction(
    const std::vector<std::string>& possible_actions,
    int call_amount,
    int min_raise,
    int max_raise) {

    if (possible_actions.empty()) {
        throw std::invalid_argument("possible_actions must not be empty");
    }

    // Choose a random action from the list
    std::uniform_int_distribution<> action_dist(0, possible_actions.size() - 1);
    std::string action = possible_actions[action_dist(rng_)];

    int amount = 0;
    if (action == "raise") {
        if (min_raise > max_raise) {
            throw std::invalid_argument("min_raise cannot exceed max_raise");
        }
        std::uniform_int_distribution<> raise_dist(min_raise, max_raise);
        amount = raise_dist(rng_);
    } else if (action == "call") {
        amount = call_amount;
    } else if (action == "fold") {
        amount = 0;
    } else {
        throw std::runtime_error("unknown action: " + action);
    }

    return {action, amount};
}