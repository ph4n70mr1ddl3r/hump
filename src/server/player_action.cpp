#include "player_action.hpp"
#include "../core/models/player.hpp"
#include "../core/hand.hpp"
#include <algorithm>
#include <chrono>

namespace player_action {

bool validateAction(const Hand& hand, const Player& player, const std::string& action, int amount) {
    if (!canAct(hand, player)) {
        return false;
    }

    if (action != "fold" && action != "call" && action != "raise") {
        return false;
    }
    if (amount < 0) {
        return false;
    }

    // Fold action must have amount of 0
    if (action == "fold" && amount != 0) {
        return false;
    }

    // Validate player stack invariants
    if (player.stack < 0) {
        return false;
    }

    // Check player has enough stack for call/raise
    if (action == "call" || action == "raise") {
        if (amount > player.stack) {
            return false;
        }
    }

    // Validate call amount is correct (must match difference to highest bet)
    if (action == "call") {
        // Find the highest bet among all players (excluding current player)
        int max_bet = 0;
        for (size_t i = 0; i < hand.player_bets.size() && i < hand.players.size(); ++i) {
            if (hand.players[i] == &player) {
                continue; // skip current player
            }
            if (hand.player_bets[i] > max_bet) {
                max_bet = hand.player_bets[i];
            }
        }
        
        // Get current player's current bet
        int player_current_bet = 0;
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == &player && i < hand.player_bets.size()) {
                player_current_bet = hand.player_bets[i];
                break;
            }
        }
        
        int required_call = max_bet - player_current_bet;
        
        // Allow exact call amount OR zero (check) when facing no bet
        if (amount != required_call && !(amount == 0 && required_call == 0)) {
            return false;
        }
    }

    // Raise must be at least min raise (total bet must be at least min_raise)
    if (action == "raise") {
        // Find current player's current bet
        int player_current_bet = 0;
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == &player && i < hand.player_bets.size()) {
                player_current_bet = hand.player_bets[i];
                break;
            }
        }
        
        // The total bet after raising must be at least min_raise
        int total_bet_after_raise = player_current_bet + amount;
        if (total_bet_after_raise < hand.min_raise) {
            return false;
        }
    }

    // Player must be current player to act
    if (hand.current_player_to_act != &player) {
        return false;
    }
    return true;
}

bool applyAction(Hand& hand, Player& player, const std::string& action, int amount) {
    if (!validateAction(hand, player, action, amount)) {
        return false;
    }

    if (action == "fold") {
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == &player) {
                if (hand.folded.size() <= i) hand.folded.resize(i + 1, false);
                hand.folded[i] = true;
                break;
            }
        }
    } else if (action == "call") {
        player.stack -= amount;
        hand.pot += amount;
    } else if (action == "raise") {
        player.stack -= amount;
        hand.pot += amount;
        int current_max_bet = 0;
        for (size_t i = 0; i < hand.player_bets.size() && i < hand.players.size(); ++i) {
            if (hand.player_bets[i] > current_max_bet) {
                current_max_bet = hand.player_bets[i];
            }
        }
        int player_current_bet = 0;
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == &player && i < hand.player_bets.size()) {
                player_current_bet = hand.player_bets[i];
                break;
            }
        }
        int new_total_bet = player_current_bet + amount;
        int raise_size = new_total_bet - current_max_bet;
        hand.min_raise = raise_size;
    }

    // Update player's total bet amount
    if (action != "fold") {
        // Find player index
        size_t player_index = hand.players.size();
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == &player) {
                player_index = i;
                break;
            }
        }
        if (player_index < hand.players.size()) {
            // Ensure player_bets vector is large enough
            if (hand.player_bets.size() <= player_index) {
                hand.player_bets.resize(player_index + 1, 0);
            }
            hand.player_bets[player_index] += amount;
        }
    }

    // Record action history
    ActionHistory history_entry;
    history_entry.player = &player;
    history_entry.action = action;
    history_entry.amount = amount;
    history_entry.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    hand.history.push_back(history_entry);

    // Advance current player (simplified - heads-up only, doesn't account for folded players or round transitions)
    // In a full NLHE implementation, this would:
    // 1. Skip folded players
    // 2. Check if betting round is complete
    // 3. Handle big blind preflop special case
    // 4. Rotate to next active player considering all betting rules
    if (hand.players.size() == 2) {
        // Check if current betting round is complete
        if (hand.isBettingRoundComplete() && hand.current_betting_round != BettingRound::SHOWDOWN) {
            BettingRound previous_round = hand.current_betting_round;
            if (hand.current_betting_round == BettingRound::PREFLOP) {
                hand.current_betting_round = BettingRound::FLOP;
                for (int i = 0; i < 3; ++i) {
                    if (!hand.deck.empty()) {
                        hand.community_cards.push_back(hand.deck.deal());
                    }
                }
            } else if (hand.current_betting_round == BettingRound::FLOP) {
                hand.current_betting_round = BettingRound::TURN;
                if (!hand.deck.empty()) {
                    hand.community_cards.push_back(hand.deck.deal());
                }
            } else if (hand.current_betting_round == BettingRound::TURN) {
                hand.current_betting_round = BettingRound::RIVER;
                if (!hand.deck.empty()) {
                    hand.community_cards.push_back(hand.deck.deal());
                }
            } else if (hand.current_betting_round == BettingRound::RIVER) {
                hand.current_betting_round = BettingRound::SHOWDOWN;
            }
            hand.last_advanced_round = previous_round;
            for (size_t i = 0; i < hand.player_bets.size(); ++i) {
                hand.player_bets[i] = 0;
            }
            if (!hand.players.empty()) {
                if (hand.current_betting_round == BettingRound::FLOP) {
                    size_t small_blind_idx = (hand.table && hand.table->dealer_button_position == 0) ? 1 : 0;
                    if (small_blind_idx < hand.players.size()) {
                        hand.current_player_to_act = hand.players[small_blind_idx];
                    }
                } else {
                    hand.current_player_to_act = hand.players[0];
                }
            }
        } else {
            hand.last_advanced_round = BettingRound::SHOWDOWN;
            hand.current_player_to_act = (hand.current_player_to_act == hand.players[0]) ? hand.players[1] : hand.players[0];
        }
    }

    return true;
}

bool canAct(const Hand& hand, const Player& player) {
    if (player.connection_status != ConnectionStatus::CONNECTED) {
        return false;
    }
    if (player.is_sitting_out) {
        return false;
    }
    for (size_t i = 0; i < hand.players.size(); ++i) {
        if (hand.players[i] == &player) {
            if (i < hand.folded.size() && hand.folded[i]) {
                return false;
            }
            return true;
        }
    }
    return false;
}

int getMinRaise(const Hand& hand) {
    return hand.min_raise;
}

int getMaxRaise(const Hand& hand, const Player& player) {
    return player.stack;
}

} // namespace player_action