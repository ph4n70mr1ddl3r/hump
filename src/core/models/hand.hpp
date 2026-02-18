#pragma once

#include "../card.hpp"
#include "../deck.hpp"
#include "table.hpp"
#include <vector>
#include <string>
#include <cstdint>

// Forward declarations
class Player;
class Table;

enum class BettingRound {
    PREFLOP,
    FLOP,
    TURN,
    RIVER,
    SHOWDOWN
};

struct ActionHistory {
    Player* player;
    std::string action; // "fold", "call", "raise"
    int amount;
    uint64_t timestamp;
};

struct Hand {
    std::string id; // UUID
    Table* table; // reference to Table
    std::vector<Player*> players; // list of Player references participating
    Deck deck; // shuffled deck for this hand
    std::vector<Card> community_cards; // array of 0-5 cards
    int pot; // total chips in main pot
    std::vector<SidePot> side_pots; // list of side pots
    std::vector<int> player_bets; // total chips contributed by each player (aligned with players)
    std::vector<bool> folded; // whether each player has folded (aligned with players)
    BettingRound current_betting_round;
    Player* current_player_to_act; // whose turn it is
    int min_raise; // minimum raise amount
    std::vector<ActionHistory> history;
    std::vector<Player*> winners; // populated at showdown
    uint64_t completed_at; // timestamp when hand finished

    // Validation helper
    bool isValid() const {
        return players.size() == 2 &&
               community_cards.size() <= 5 &&
               min_raise >= 4; // at least big blind
    }

    // Check if betting round is complete
    bool isBettingRoundComplete() const {
        if (players.empty() || folded.empty() || player_bets.empty()) {
            return false;
        }

        // Count active (non-folded) players
        int active_players = 0;
        for (size_t i = 0; i < players.size(); ++i) {
            if (i < folded.size() && !folded[i]) {
                active_players++;
            }
        }

        // If only one player remains, round is complete
        if (active_players <= 1) {
            return true;
        }

        // Check if all active players have equal bets
        int target_bet = -1;
        for (size_t i = 0; i < players.size(); ++i) {
            if (i < folded.size() && !folded[i]) {
                if (i >= player_bets.size()) {
                    return false;
                }
                if (target_bet == -1) {
                    target_bet = player_bets[i];
                } else if (player_bets[i] != target_bet) {
                    return false;
                }
            }
        }

        return target_bet >= 0;
    }

    // Get current player to act
    Player* getCurrentPlayer() const {
        return current_player_to_act;
    }

    // Advance to next betting round
    bool advanceRound() {
        switch (current_betting_round) {
            case BettingRound::PREFLOP:
                current_betting_round = BettingRound::FLOP;
                return true;
            case BettingRound::FLOP:
                current_betting_round = BettingRound::TURN;
                return true;
            case BettingRound::TURN:
                current_betting_round = BettingRound::RIVER;
                return true;
            case BettingRound::RIVER:
                current_betting_round = BettingRound::SHOWDOWN;
                return true;
            default:
                return false;
        }
    }
};
