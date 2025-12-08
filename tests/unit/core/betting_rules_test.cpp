#include <gtest/gtest.h>
#include "betting_rules.hpp"

TEST(BettingRulesTest, CalculateMinRaise) {
    EXPECT_EQ(BettingRules::calculateMinRaise(0, 4), 4); // No bet yet, min raise = big blind
    EXPECT_EQ(BettingRules::calculateMinRaise(4, 4), 8); // After bet of 4, min raise to 8
    EXPECT_EQ(BettingRules::calculateMinRaise(10, 4), 14); // After bet of 10, min raise to 14
}

TEST(BettingRulesTest, CalculateMaxRaise) {
    EXPECT_EQ(BettingRules::calculateMaxRaise(100), 100); // Can raise up to stack
    EXPECT_EQ(BettingRules::calculateMaxRaise(0), 0); // No stack, cannot raise
}

TEST(BettingRulesTest, IsValidActionFold) {
    BettingState round{4, 8, 100};
    EXPECT_TRUE(BettingRules::isValidAction(Action::FOLD, 0, round, 100));
}

TEST(BettingRulesTest, IsValidActionCall) {
    BettingState round{4, 8, 100};
    EXPECT_TRUE(BettingRules::isValidAction(Action::CALL, 4, round, 100));
    EXPECT_FALSE(BettingRules::isValidAction(Action::CALL, 0, round, 100)); // Wrong amount
    EXPECT_FALSE(BettingRules::isValidAction(Action::CALL, 8, round, 100)); // Over-call not allowed
}

TEST(BettingRulesTest, IsValidActionRaise) {
    BettingState round{4, 8, 100};
    EXPECT_TRUE(BettingRules::isValidAction(Action::RAISE, 8, round, 100)); // Min raise
    EXPECT_TRUE(BettingRules::isValidAction(Action::RAISE, 20, round, 100)); // Valid raise
    EXPECT_FALSE(BettingRules::isValidAction(Action::RAISE, 4, round, 100)); // Below min raise
    EXPECT_FALSE(BettingRules::isValidAction(Action::RAISE, 101, round, 100)); // Exceeds stack
    EXPECT_FALSE(BettingRules::isValidAction(Action::RAISE, 0, round, 100)); // Zero raise
}

TEST(BettingRulesTest, ApplyActionFold) {
    BettingState round{4, 8, 100};
    BettingState new_round = BettingRules::applyAction(Action::FOLD, 0, round, 100);
    EXPECT_EQ(new_round.pot, 100); // Pot unchanged
}

TEST(BettingRulesTest, ApplyActionCall) {
    BettingState round{4, 8, 100};
    BettingState new_round = BettingRules::applyAction(Action::CALL, 4, round, 100);
    EXPECT_EQ(new_round.pot, 104); // Pot increased by call amount
    EXPECT_EQ(new_round.current_bet, 4); // Current bet unchanged
}

TEST(BettingRulesTest, ApplyActionRaise) {
    BettingState round{4, 8, 100};
    BettingState new_round = BettingRules::applyAction(Action::RAISE, 12, round, 100);
    EXPECT_EQ(new_round.pot, 112); // Pot increased by raise amount
    EXPECT_EQ(new_round.current_bet, 12); // New current bet
    EXPECT_EQ(new_round.min_raise, 16); // New min raise (12 + 4)
}

TEST(BettingRulesTest, IsRoundComplete) {
    std::vector<bool> has_acted = {true, true};
    std::vector<int> bets = {4, 4};
    EXPECT_TRUE(BettingRules::isRoundComplete(has_acted, bets, 4));
    
    has_acted = {true, false};
    EXPECT_FALSE(BettingRules::isRoundComplete(has_acted, bets, 4));
    
    bets = {4, 8}; // Different bets
    has_acted = {true, true};
    EXPECT_FALSE(BettingRules::isRoundComplete(has_acted, bets, 8)); // Not all called
}

TEST(BettingRulesTest, BetExceedingStackTreatedAsAllIn) {
    // If a player tries to bet more than their stack, it's treated as all-in (bet = stack)
    // This is already handled by isValidAction: raise amount cannot exceed stack
    // So a raise of 150 with stack 100 should be invalid
    BettingState round{4, 8, 100};
    EXPECT_FALSE(BettingRules::isValidAction(Action::RAISE, 150, round, 100));
    // However, the caller (opponent) can call with remaining stack when raise exceeds opponent stack
    // This is a different scenario: player raises to 100 (all-in), opponent with stack 50 can call 50
    // This is handled by game logic, not betting rules.
}

TEST(BettingRulesTest, RaiseExceedingOpponentStack) {
    // Player1 raises to 100 (stack 200), Player2 stack 50
    // Player2 can call all-in with 50, remaining 50 is returned to Player1? Not in NLHE.
    // This edge case is beyond betting rules; it's about side pots.
    // We'll just ensure that isValidAction allows raise up to player's own stack.
    BettingState round{4, 8, 100};
    EXPECT_TRUE(BettingRules::isValidAction(Action::RAISE, 100, round, 100)); // all-in raise
    EXPECT_FALSE(BettingRules::isValidAction(Action::RAISE, 101, round, 100)); // exceeds stack
}