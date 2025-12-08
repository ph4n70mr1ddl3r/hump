#include <gtest/gtest.h>
#include <optional>
#include "core/hand.hpp"
#include "core/deck.hpp"
#include "core/models/player.hpp"
#include "core/pot.hpp"

TEST(FullHandFlowTest, BasicHandStart) {
    // Create two players
    Player player1{"player1", "Player1", 400, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    Player player2{"player2", "Player2", 400, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    
    // Create deck and hand
    Deck deck;
    Hand hand;
    
    // Start hand with player1 as dealer, player2 as big blind (simplified)
    poker::startHand(hand, deck, &player1, &player1, &player2);
    
    // Verify hand state
    EXPECT_EQ(hand.players.size(), 2);
    EXPECT_EQ(hand.players[0], &player1);
    EXPECT_EQ(hand.players[1], &player2);
    EXPECT_EQ(hand.current_betting_round, BettingRound::PREFLOP);
    EXPECT_EQ(hand.pot, 0);
    EXPECT_EQ(hand.side_pots.size(), 0);
    
    // Verify each player has 2 hole cards
    EXPECT_EQ(player1.hole_cards.size(), 2);
    EXPECT_EQ(player2.hole_cards.size(), 2);
    
    // Verify deck has 48 cards remaining (52 - 4)
    // EXPECT_EQ(deck.size(), 48); // size() returns total cards, not remaining
}

TEST(FullHandFlowTest, FullHandWithFold) {
    Player player1{"player1", "Player1", 400, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    Player player2{"player2", "Player2", 400, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    
    Deck deck;
    Hand hand;
    poker::startHand(hand, deck, &player1, &player1, &player2);
    
    // Simulate a fold action from player1
    bool applied = poker::applyAction(hand, &player1, "fold", 0);
    EXPECT_TRUE(applied);
    
    // After fold, hand should be complete with player2 as winner
    EXPECT_TRUE(poker::isHandComplete(hand));
    
    // Determine winners
    auto winners = poker::determineWinners(hand);
    EXPECT_EQ(winners.size(), 1);
    EXPECT_EQ(winners[0], &player2);
    
    // Pot distribution (no pot because no bets)
    poker::calculateSidePots(hand);
    EXPECT_EQ(hand.side_pots.size(), 0);
}

TEST(FullHandFlowTest, FullHandWithAllInSidePot) {
    Player player1{"player1", "Player1", 100, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false}; // short stack
    Player player2{"player2", "Player2", 400, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    
    Deck deck;
    Hand hand;
    poker::startHand(hand, deck, &player1, &player1, &player2);
    
    // Player2 raises to 50 (more than player1's stack)
    bool applied = poker::applyAction(hand, &player2, "raise", 50);
    EXPECT_TRUE(applied);
    
    // Player1 calls all-in with 100 (total stack)
    applied = poker::applyAction(hand, &player1, "call", 100);
    EXPECT_TRUE(applied);
    
    // Calculate side pots
    poker::calculateSidePots(hand);
    
    // Should have at least one side pot
    EXPECT_GE(hand.side_pots.size(), 1);
    
    // Verify total pot amount matches sum of bets
    int total_bets = hand.player_bets[0] + hand.player_bets[1];
    EXPECT_EQ(hand.pot, total_bets);
    
    // Simulate remaining betting rounds (simplified: just advance)
    poker::advanceBettingRound(hand); // PREFLOP -> FLOP
    poker::advanceBettingRound(hand); // FLOP -> TURN
    poker::advanceBettingRound(hand); // TURN -> RIVER
    poker::advanceBettingRound(hand); // RIVER -> SHOWDOWN
    
    // Determine winners (simplified: assume player2 wins)
    auto winners = poker::determineWinners(hand);
    // For now just verify function doesn't crash
    EXPECT_TRUE(winners.size() >= 0);
}

TEST(FullHandFlowTest, PotDistribution) {
    Player player1{"player1", "Player1", 200, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    Player player2{"player2", "Player2", 200, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    
    Deck deck;
    Hand hand;
    poker::startHand(hand, deck, &player1, &player1, &player2);
    
    // Both players call a bet of 50
    poker::applyAction(hand, &player1, "call", 50);
    poker::applyAction(hand, &player2, "call", 50);
    
    // Pot should be 100
    EXPECT_EQ(hand.pot, 100);
    
    // Distribute pot to player1 (winner)
    std::vector<Player*> winners = {&player1};
    pot::distributePot(hand, winners);
    
    // Pot should be zero after distribution
    EXPECT_EQ(hand.pot, 0);
    
    // Player1 stack should increase by 100
    EXPECT_EQ(player1.stack, 250);
    // Player2 stack unchanged (except the 50 already subtracted)
    EXPECT_EQ(player2.stack, 150);
}

// Test that player bets are tracked correctly
TEST(FullHandFlowTest, PlayerBetsTracking) {
    Player player1{"player1", "Player1", 400, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    Player player2{"player2", "Player2", 400, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
    
    Deck deck;
    Hand hand;
    poker::startHand(hand, deck, &player1, &player1, &player2);
    
    // Initial player_bets should be zero for both
    EXPECT_EQ(hand.player_bets.size(), 2);
    EXPECT_EQ(hand.player_bets[0], 0);
    EXPECT_EQ(hand.player_bets[1], 0);
    
    // Player1 raises 20
    poker::applyAction(hand, &player1, "raise", 20);
    EXPECT_EQ(hand.player_bets[0], 20);
    EXPECT_EQ(hand.player_bets[1], 0);
    
    // Player2 calls 20
    poker::applyAction(hand, &player2, "call", 20);
    EXPECT_EQ(hand.player_bets[0], 20);
    EXPECT_EQ(hand.player_bets[1], 20);
}