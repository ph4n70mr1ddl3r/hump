#include <gtest/gtest.h>
#include "random_strategy.hpp"
#include <vector>
#include <string>

TEST(RandomStrategyTest, ChooseActionValidActions) {
    RandomStrategy strategy;
    std::vector<std::string> possible_actions = {"fold", "call", "raise"};
    int call_amount = 10;
    int min_raise = 20;
    int max_raise = 100;
    
    // Run multiple times to ensure random choices are valid
    for (int i = 0; i < 100; ++i) {
        auto [action, amount] = strategy.chooseAction(possible_actions, call_amount, min_raise, max_raise);
        EXPECT_TRUE(action == "fold" || action == "call" || action == "raise");
        if (action == "fold") {
            EXPECT_EQ(amount, 0);
        } else if (action == "call") {
            EXPECT_EQ(amount, call_amount);
        } else if (action == "raise") {
            EXPECT_GE(amount, min_raise);
            EXPECT_LE(amount, max_raise);
        }
    }
}

TEST(RandomStrategyTest, ChooseActionOnlyFold) {
    RandomStrategy strategy;
    std::vector<std::string> possible_actions = {"fold"};
    auto [action, amount] = strategy.chooseAction(possible_actions, 10, 20, 100);
    EXPECT_EQ(action, "fold");
    EXPECT_EQ(amount, 0);
}

TEST(RandomStrategyTest, ChooseActionOnlyCall) {
    RandomStrategy strategy;
    std::vector<std::string> possible_actions = {"call"};
    auto [action, amount] = strategy.chooseAction(possible_actions, 15, 20, 100);
    EXPECT_EQ(action, "call");
    EXPECT_EQ(amount, 15);
}

TEST(RandomStrategyTest, ChooseActionOnlyRaise) {
    RandomStrategy strategy;
    std::vector<std::string> possible_actions = {"raise"};
    int min_raise = 30;
    int max_raise = 50;
    auto [action, amount] = strategy.chooseAction(possible_actions, 10, min_raise, max_raise);
    EXPECT_EQ(action, "raise");
    EXPECT_GE(amount, min_raise);
    EXPECT_LE(amount, max_raise);
}

TEST(RandomStrategyTest, ChooseActionRaiseRangeSingleValue) {
    RandomStrategy strategy;
    std::vector<std::string> possible_actions = {"raise"};
    int min_raise = 25;
    int max_raise = 25;
    auto [action, amount] = strategy.chooseAction(possible_actions, 10, min_raise, max_raise);
    EXPECT_EQ(action, "raise");
    EXPECT_EQ(amount, 25);
}

TEST(RandomStrategyTest, ChooseActionEmptyVectorThrows) {
    RandomStrategy strategy;
    std::vector<std::string> possible_actions;
    EXPECT_THROW(strategy.chooseAction(possible_actions, 10, 20, 100), std::invalid_argument);
}

TEST(RandomStrategyTest, ChooseActionInvalidRaiseRangeThrows) {
    RandomStrategy strategy;
    std::vector<std::string> possible_actions = {"raise"};
    // min_raise > max_raise should throw
    EXPECT_THROW(strategy.chooseAction(possible_actions, 10, 100, 50), std::invalid_argument);
}