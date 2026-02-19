#include "hand.hpp"
#include "models/player.hpp"
#include "hand_ranking.hpp"
#include "pot.hpp"
#include "../common/constants.hpp"
#include "../common/uuid.hpp"
#include <algorithm>
#include <stdexcept>
#include <cstdlib>
#include <chrono>

namespace poker {

void startHand(Hand& hand, Deck& deck, Player* dealer, Player* small_blind, Player* big_blind) {
    // Reset hand state
    hand.id = "hand_" + common::uuid::generate();
    hand.table = nullptr; // caller should set
    hand.players = {small_blind, big_blind};
    hand.player_bets.resize(hand.players.size(), 0);
    hand.folded.resize(hand.players.size(), false);
    hand.deck = deck;
    hand.deck.shuffle();
    hand.community_cards.clear();
    hand.pot = 0;
    hand.side_pots.clear();
    hand.current_betting_round = BettingRound::PREFLOP;
    hand.current_player_to_act = small_blind;
    hand.min_raise = common::constants::BIG_BLIND;
    hand.history.clear();
    hand.winners.clear();
    hand.completed_at = 0;

    // Post blinds
    int small_blind_amount = common::constants::SMALL_BLIND;
    int big_blind_amount = common::constants::BIG_BLIND;

    if (small_blind && small_blind->stack >= small_blind_amount) {
        small_blind->stack -= small_blind_amount;
        hand.pot += small_blind_amount;
        hand.player_bets[0] = small_blind_amount;
    }
    if (big_blind && big_blind->stack >= big_blind_amount) {
        big_blind->stack -= big_blind_amount;
        hand.pot += big_blind_amount;
        hand.player_bets[1] = big_blind_amount;
    }

    // Deal hole cards
    dealHoleCards(hand, hand.deck);
}

void dealHoleCards(Hand& hand, Deck& deck) {
    for (auto player : hand.players) {
        player->hole_cards.clear();
        player->hole_cards.push_back(deck.deal());
        player->hole_cards.push_back(deck.deal());
    }
}

void dealCommunityCards(Hand& hand, Deck& deck, int count) {
    for (int i = 0; i < count; ++i) {
        hand.community_cards.push_back(deck.deal());
    }
}

bool applyAction(Hand& hand, Player* player, const std::string& action, int amount) {
    if (!player) {
        return false;
    }

    // Ensure folded and player_bets vectors are properly sized
    if (hand.folded.size() != hand.players.size()) {
        hand.folded.resize(hand.players.size(), false);
    }
    if (hand.player_bets.size() != hand.players.size()) {
        hand.player_bets.resize(hand.players.size(), 0);
    }

    // Simplified validation
    if (action == "fold") {
        // Mark player folded
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == player) {
                hand.folded[i] = true;
                break;
            }
        }
    } else if (action == "call") {
        if (amount > player->stack) return false;
        if (amount < 0) return false;
        player->stack -= amount;
        hand.pot += amount;
        // Update player's total bet amount
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == player) {
                hand.player_bets[i] += amount;
                break;
            }
        }
    } else if (action == "raise") {
        if (amount < hand.min_raise) return false;
        if (amount > player->stack) return false;
        if (amount < 0) return false;
        player->stack -= amount;
        hand.pot += amount;
        // Update player's total bet amount
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == player) {
                hand.player_bets[i] += amount;
                break;
            }
        }
        hand.min_raise = amount;
    } else {
        return false;
    }

    // Record action history
    ActionHistory history_entry;
    history_entry.player = player;
    history_entry.action = action;
    history_entry.amount = amount;
    history_entry.timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    hand.history.push_back(history_entry);

    // Move to next player (simplified)
    // In heads-up, toggle between two players
    if (hand.players.size() == 2) {
        hand.current_player_to_act = (hand.current_player_to_act == hand.players[0]) ? hand.players[1] : hand.players[0];
    }

    return true;
}

bool advanceBettingRound(Hand& hand) {
    // Use hand's built-in advanceRound method
    bool advanced = hand.advanceRound();
    if (advanced) {
        // Deal community cards based on new round
        switch (hand.current_betting_round) {
            case BettingRound::FLOP:
                dealCommunityCards(hand, hand.deck, 3);
                break;
            case BettingRound::TURN:
            case BettingRound::RIVER:
                dealCommunityCards(hand, hand.deck, 1);
                break;
            default:
                break;
        }
    }
    return advanced;
}

bool isHandComplete(const Hand& hand) {
    // Hand is complete if we have reached showdown or only one player remains
    if (hand.current_betting_round == BettingRound::SHOWDOWN) {
        return true;
    }
    // Ensure folded vector is properly sized
    size_t folded_size = hand.folded.size();
    if (folded_size != hand.players.size()) {
        folded_size = hand.players.size();
    }
    // Count players who haven't folded
    int active_players = 0;
    for (size_t i = 0; i < hand.players.size(); ++i) {
        if (i >= folded_size || !hand.folded[i]) {
            active_players++;
        }
    }
    return active_players <= 1;
}

std::vector<Player*> determineWinners(const Hand& hand) {
    std::vector<Player*> winners;
    if (hand.players.empty()) return winners;

    // Count active (non-folded) players
    std::vector<Player*> active_players;
    for (size_t i = 0; i < hand.players.size(); ++i) {
        if (i >= hand.folded.size() || !hand.folded[i]) {
            active_players.push_back(hand.players[i]);
        }
    }

    // If only one active player, they win
    if (active_players.size() == 1) {
        winners.push_back(active_players[0]);
        return winners;
    }

    // Evaluate each active player's hand (hole cards + community cards)
    std::vector<std::pair<Player*, HandRank>> evaluations;
    std::vector<std::vector<Card>> all_cards_list;
    for (auto player : active_players) {
        std::vector<Card> all_cards = player->hole_cards;
        all_cards.insert(all_cards.end(), hand.community_cards.begin(), hand.community_cards.end());
        HandRank rank = HandRanking::evaluate(all_cards);
        evaluations.emplace_back(player, rank);
        all_cards_list.push_back(std::move(all_cards));
    }

    size_t best_idx = 0;
    for (size_t i = 1; i < evaluations.size(); ++i) {
        if (evaluations[i].second > evaluations[best_idx].second) {
            best_idx = i;
        } else if (evaluations[i].second == evaluations[best_idx].second) {
            int cmp = HandRanking::compare(all_cards_list[i], all_cards_list[best_idx]);
            if (cmp > 0) {
                best_idx = i;
            }
        }
    }
    
    HandRank best_rank = evaluations[best_idx].second;
    const auto& best_cards = all_cards_list[best_idx];
    for (size_t i = 0; i < evaluations.size(); ++i) {
        if (evaluations[i].second == best_rank) {
            if (i == best_idx || HandRanking::compare(all_cards_list[i], best_cards) == 0) {
                winners.push_back(evaluations[i].first);
            }
        }
    }
    return winners;
}

void calculateSidePots(Hand& hand) {
    hand.side_pots = pot::calculateSidePots(hand.players, hand.player_bets);
}

void resetHand(Hand& hand) {
    hand.id.clear();
    hand.table = nullptr;
    hand.players.clear();
    hand.deck = Deck();
    hand.community_cards.clear();
    hand.pot = 0;
    hand.side_pots.clear();
    hand.player_bets.clear();
    hand.folded.clear();
    hand.current_betting_round = BettingRound::PREFLOP;
    hand.current_player_to_act = nullptr;
    hand.min_raise = common::constants::BIG_BLIND;
    hand.history.clear();
    hand.winners.clear();
    hand.completed_at = 0;
}

} // namespace poker