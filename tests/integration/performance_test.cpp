#include <gtest/gtest.h>
#include <optional>
#include "core/hand.hpp"
#include "core/deck.hpp"
#include "core/models/player.hpp"
#include "core/pot.hpp"

#include <random>
#include <chrono>
#include <iostream>

// Performance benchmark test for SC-008: Server achieves sub-second latency for action processing
// and supports 100+ hands/hour throughput.

namespace {

// Simulate a single hand with random actions (similar to stability test)
bool simulateRandomHand(Player& player1, Player& player2, std::mt19937& rng) {
    Deck deck;
    Hand hand;
    poker::startHand(hand, deck, &player1, &player1, &player2);
    
    int max_iterations = 1000;
    int iter = 0;
    while (!poker::isHandComplete(hand) && iter < max_iterations) {
        Player* active_player = hand.current_player_to_act;
        if (active_player == nullptr) break;
        
        // Compute possible actions
        // Simplified: always call if possible, else fold
        int max_bet = 0;
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (i < hand.player_bets.size() && hand.player_bets[i] > max_bet) {
                max_bet = hand.player_bets[i];
            }
        }
        int player_cur_bet = 0;
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == active_player && i < hand.player_bets.size()) {
                player_cur_bet = hand.player_bets[i];
                break;
            }
        }
        int call_amount = max_bet - player_cur_bet;
        if (call_amount > active_player->stack) {
            call_amount = active_player->stack;
        }
        
        std::string action;
        int amount = 0;
        if (call_amount == 0) {
            action = "call"; // check
        } else if (call_amount <= active_player->stack) {
            action = "call";
            amount = call_amount;
        } else {
            action = "fold";
        }
        
        bool applied = poker::applyAction(hand, active_player, action, amount);
        if (!applied) {
            poker::applyAction(hand, active_player, "fold", 0);
        }
        ++iter;
    }
    return poker::isHandComplete(hand);
}

} // anonymous namespace

class PerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        rng.seed(std::random_device{}());
    }

    std::mt19937 rng;
};

TEST_F(PerformanceTest, ThroughputOneHundredHandsPerHour) {
    const int NUM_HANDS = 100;
    int hands_completed = 0;
    
    auto start_time = std::chrono::steady_clock::now();
    
    for (int i = 0; i < NUM_HANDS; ++i) {
        Player player1{"player1", "Player1", 400, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
        Player player2{"player2", "Player2", 400, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
        
        bool completed = simulateRandomHand(player1, player2, rng);
        if (completed) {
            ++hands_completed;
        }
    }
    
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    double elapsed_seconds = elapsed_ms / 1000.0;
    
    // Requirement: 100+ hands per hour => each hand should take <= 36 seconds on average
    // This test verifies that 100 hands complete in less than 3600 seconds (1 hour)
    const double max_total_seconds = 3600.0; // 1 hour
    EXPECT_LE(elapsed_seconds, max_total_seconds) << "Throughput too low: " << elapsed_seconds << " seconds for " << NUM_HANDS << " hands";
    
    double avg_ms_per_hand = elapsed_ms / static_cast<double>(NUM_HANDS);
    // Sub-second latency requirement (less than 1000 ms per hand)
    EXPECT_LE(avg_ms_per_hand, 1000.0) << "Average latency per hand exceeds 1 second: " << avg_ms_per_hand << " ms";
    
    std::cout << "[ INFO ] Performance benchmark: " << hands_completed << " hands in " << elapsed_ms << " ms" << std::endl;
    std::cout << "[ INFO ] Average latency per hand: " << avg_ms_per_hand << " ms" << std::endl;
    std::cout << "[ INFO ] Estimated hands per hour: " << (3600.0 / elapsed_seconds * NUM_HANDS) << std::endl;
}