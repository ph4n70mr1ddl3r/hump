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
    hand.current_player_to_act = small_blind; // small blind acts first preflop? Actually big blind acts last preflop. Simplified.
    hand.min_raise = common::constants::BIG_BLIND;
    hand.history.clear();
    hand.winners.clear();
    hand.completed_at = 0;

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

    // Simplified validation
    if (action == "fold") {
        // Mark player folded
        for (size_t i = 0; i < hand.players.size(); ++i) {
            if (hand.players[i] == player) {
                if (hand.folded.size() <= i) hand.folded.resize(i + 1, false);
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
                if (hand.player_bets.size() <= i) hand.player_bets.resize(i + 1, 0);
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
                if (hand.player_bets.size() <= i) hand.player_bets.resize(i + 1, 0);
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
    // Count players who haven't folded
    int active_players = 0;
    size_t folded_size = hand.folded.size();
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
    for (auto player : active_players) {
        // Combine cards
        std::vector<Card> all_cards = player->hole_cards;
        all_cards.insert(all_cards.end(), hand.community_cards.begin(), hand.community_cards.end());
        HandRank rank = HandRanking::evaluate(all_cards);
        evaluations.emplace_back(player, rank);
    }

    // Find max rank
    auto max_it = std::max_element(evaluations.begin(), evaluations.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });
    if (max_it != evaluations.end()) {
        winners.push_back(max_it->first);
    }
    return winners;
}

void calculateSidePots(Hand& hand) {
    hand.side_pots = pot::calculateSidePots(hand.players, hand.player_bets);
}

void updatePot(Hand& hand, Player* player, int amount) {
    hand.pot += amount;
}

void resetHand(Hand& hand) {
    hand.id.clear();
    hand.table = nullptr;
    hand.players.clear();
    hand.deck = Deck();
    hand.deck.shuffle();
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