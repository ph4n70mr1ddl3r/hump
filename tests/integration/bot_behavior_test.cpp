#include <gtest/gtest.h>
#include "random_strategy.hpp"
#include "delay.hpp"
#include "stack_management.hpp"
#include <vector>
#include <string>

TEST(BotBehaviorIntegrationTest, RandomStrategyChoosesValidActions) {
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

TEST(BotBehaviorIntegrationTest, DelayDoesNotThrow) {
    // Ensure delay function runs without throwing
    EXPECT_NO_THROW(delay::randomDelay(10, 20));
}

TEST(BotBehaviorIntegrationTest, StackManagementThreshold) {
    using namespace stack_management;
    EXPECT_TRUE(shouldTopUp(0));
    EXPECT_TRUE(shouldTopUp(19));
    EXPECT_FALSE(shouldTopUp(20));
    EXPECT_FALSE(shouldTopUp(400));
}

TEST(BotBehaviorIntegrationTest, StackTopUpAmount) {
    using namespace stack_management;
    EXPECT_EQ(topUpAmount(0), 400);
    EXPECT_EQ(topUpAmount(10), 390);
    EXPECT_EQ(topUpAmount(400), 0);
    EXPECT_EQ(topUpAmount(401), 0);
}

TEST(BotBehaviorIntegrationTest, StackTopUp) {
    using namespace stack_management;
    EXPECT_EQ(topUp(0), 400);
    EXPECT_EQ(topUp(10), 400);
    EXPECT_EQ(topUp(400), 400);
    EXPECT_EQ(topUp(401), 401);
}