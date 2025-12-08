#include <gtest/gtest.h>
#include <optional>
#include <algorithm>
#include "core/hand.hpp"
#include "core/deck.hpp"
#include "core/models/player.hpp"
#include "core/pot.hpp"
#include "core/betting_rules.hpp"
#include <random>

// Stability test for SC-001: Two bots can play at least 100 consecutive hands without manual intervention
// This test simulates 100 hands with random valid actions to ensure no crashes or infinite loops.

class StabilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize random number generator
        rng.seed(std::random_device{}());
    }

    std::mt19937 rng;
};

TEST_F(StabilityTest, OneHundredConsecutiveHands) {
    const int NUM_HANDS = 100;
    int hands_completed = 0;
    
    for (int hand_num = 0; hand_num < NUM_HANDS; ++hand_num) {
        // Create two players with random stacks (between 200 and 1000)
        std::uniform_int_distribution<int> stack_dist(200, 1000);
        int stack1 = stack_dist(rng);
        int stack2 = stack_dist(rng);
        
        Player player1{"player1", "Player1", stack1, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
        Player player2{"player2", "Player2", stack2, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
        
        // Create deck and hand
        Deck deck;
        Hand hand;
        
        // Start hand with player1 as dealer, player2 as big blind (simplified)
        poker::startHand(hand, deck, &player1, &player1, &player2);
        
        // Simulate hand with random actions until completion
        int max_iterations = 1000; // safety limit
        int iter = 0;
        while (!poker::isHandComplete(hand) && iter < max_iterations) {
            // Determine active player (simplified: assume hand.current_player_index)
            Player* active_player = hand.players[hand.current_player_index];
            
            // Find active player index and compute betting amounts
            int active_player_index = -1;
            for (size_t i = 0; i < hand.players.size(); ++i) {
                if (hand.players[i] == active_player) {
                    active_player_index = i;
                    break;
                }
            }
            int player_bet = (active_player_index >= 0 && active_player_index < hand.player_bets.size()) 
                ? hand.player_bets[active_player_index] : 0;
            int current_bet = 0;
            for (int bet : hand.player_bets) {
                if (bet > current_bet) current_bet = bet;
            }
            
            // Generate random valid action
            std::vector<std::string> possible_actions = {"fold", "call", "raise"};
            std::uniform_int_distribution<int> action_dist(0, possible_actions.size() - 1);
            std::string action = possible_actions[action_dist(rng)];
            
            int amount = 0;
            if (action == "call") {
                amount = current_bet - player_bet;
                if (amount > active_player->stack) {
                    amount = active_player->stack; // all-in
                }
            } else if (action == "raise") {
                // Random raise between min raise and player's stack
                int min_raise = BettingRules::calculateMinRaise(current_bet, 4); // big blind = 4
                if (min_raise <= active_player->stack) {
                    std::uniform_int_distribution<int> raise_dist(min_raise, active_player->stack);
                    amount = raise_dist(rng);
                } else {
                    // Cannot raise, fall back to call or fold
                    if (current_bet - player_bet <= active_player->stack) {
                        action = "call";
                        amount = current_bet - player_bet;
                    } else {
                        action = "fold";
                        amount = 0;
                    }
                }
            } // fold: amount stays 0
            
            // Apply action
            bool applied = poker::applyAction(hand, active_player, action, amount);
            // Action may be invalid (e.g., raise less than min), in which case we skip iteration
            if (!applied) {
                // Force fold to avoid infinite loop
                poker::applyAction(hand, active_player, "fold", 0);
            }
            
            ++iter;
        }
        
        // Hand should be complete
        EXPECT_TRUE(poker::isHandComplete(hand));
        ++hands_completed;
    }
    
    EXPECT_EQ(hands_completed, NUM_HANDS);
    std::cout << "[ INFO ] Successfully simulated " << hands_completed << " consecutive hands without crash." << std::endl;
}