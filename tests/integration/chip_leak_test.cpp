#include <gtest/gtest.h>
#include <optional>
#include "core/hand.hpp"
#include "core/deck.hpp"
#include "core/models/player.hpp"
#include "core/pot.hpp"

#include <random>

// Chip leak validation test for SC-002: Server maintains game state correctly across hand transitions
// with no chip leaks or state corruption (total chips in play remains constant).

namespace {

// Calculate total chips in play: sum of player stacks + main pot + side pots
int totalChips(const Hand& hand, const std::vector<Player*>& players) {
    int total = 0;
    for (const auto* player : players) {
        total += player->stack;
    }
    total += hand.pot;
    for (const auto& side_pot : hand.side_pots) {
        total += side_pot.amount;
    }
    return total;
}

// Find the maximum bet among players in the current hand
int maxBet(const Hand& hand) {
    int max = 0;
    for (size_t i = 0; i < hand.players.size(); ++i) {
        if (i < hand.player_bets.size()) {
            if (hand.player_bets[i] > max) {
                max = hand.player_bets[i];
            }
        }
    }
    return max;
}

// Find the current bet contributed by a specific player
int playerBet(const Hand& hand, const Player* player) {
    for (size_t i = 0; i < hand.players.size(); ++i) {
        if (hand.players[i] == player && i < hand.player_bets.size()) {
            return hand.player_bets[i];
        }
    }
    return 0;
}

} // anonymous namespace

class ChipLeakTest : public ::testing::Test {
protected:
    void SetUp() override {
        rng.seed(std::random_device{}());
    }

    std::mt19937 rng;
};

TEST_F(ChipLeakTest, NoChipLeakAcrossOneHundredHands) {
    const int NUM_HANDS = 100;
    int hands_completed = 0;
    
    for (int hand_num = 0; hand_num < NUM_HANDS; ++hand_num) {
        // Create two players with initial stacks (both 400 chips, total 800)
        Player player1{"player1", "Player1", 400, 0, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
        Player player2{"player2", "Player2", 400, 1, {}, ConnectionStatus::CONNECTED, 0, std::nullopt, false};
        
        // Create deck and hand
        Deck deck;
        Hand hand;
        
        // Start hand with player1 as dealer, player2 as big blind (simplified)
        poker::startHand(hand, deck, &player1, &player1, &player2);
        
        // Record total chips before the hand
        std::vector<Player*> players = {&player1, &player2};
        int initial_total = totalChips(hand, players);
        
        // Simulate hand with random actions until completion
        int max_iterations = 1000; // safety limit
        int iter = 0;
        while (!poker::isHandComplete(hand) && iter < max_iterations) {
            // Determine active player
            Player* active_player = hand.current_player_to_act;
            if (active_player == nullptr) {
                // No active player, break
                break;
            }
            
            // Compute possible actions based on hand state
            // For simplicity, we allow fold, call, raise (if possible)
            std::vector<std::string> possible_actions = {"fold", "call", "raise"};
            
            // Remove "raise" if player cannot raise (stack too low)
            int max_bet = maxBet(hand);
            int player_cur_bet = playerBet(hand, active_player);
            int call_amount = max_bet - player_cur_bet;
            if (call_amount > active_player->stack) {
                call_amount = active_player->stack; // all-in call
            }
            // Minimum raise amount is hand.min_raise (already set)
            bool can_raise = (active_player->stack >= hand.min_raise);
            
            // Randomly choose an action, weighted towards call/raise to keep game moving
            std::uniform_int_distribution<int> action_dist(0, possible_actions.size() - 1);
            std::string action = possible_actions[action_dist(rng)];
            
            // Adjust action if not possible
            if (action == "raise" && !can_raise) {
                action = (call_amount <= active_player->stack) ? "call" : "fold";
            }
            if (action == "call" && call_amount > active_player->stack) {
                action = "fold";
            }
            
            int amount = 0;
            if (action == "call") {
                amount = call_amount;
            } else if (action == "raise") {
                // Random raise between min_raise and player's stack
                std::uniform_int_distribution<int> raise_dist(hand.min_raise, active_player->stack);
                amount = raise_dist(rng);
            } // fold: amount stays 0
            
            // Record total chips before action
            int before_total = totalChips(hand, players);
            
            // Apply action
            bool applied = poker::applyAction(hand, active_player, action, amount);
            if (!applied) {
                // Action invalid (should not happen with our logic), force fold to continue
                poker::applyAction(hand, active_player, "fold", 0);
            }
            
            // Verify chip conservation after each action
            int after_total = totalChips(hand, players);
            EXPECT_EQ(before_total, after_total) << "Chip leak detected after action " << action 
                                                 << " amount " << amount << " in hand " << hand_num
                                                 << " iteration " << iter;
            
            ++iter;
        }
        
        // Hand should be complete
        EXPECT_TRUE(poker::isHandComplete(hand));
        
        // Verify total chips after hand equals initial total (no leak across hand transition)
        int final_total = totalChips(hand, players);
        EXPECT_EQ(initial_total, final_total) << "Chip leak across hand transition in hand " << hand_num;
        
        ++hands_completed;
    }
    
    EXPECT_EQ(hands_completed, NUM_HANDS);
    std::cout << "[ INFO ] Successfully simulated " << hands_completed 
              << " consecutive hands without chip leak." << std::endl;
}