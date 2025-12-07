#include "pot.hpp"
#include "models/player.hpp"
#include "models/table.hpp"
#include "models/hand.hpp"
#include "hand_ranking.hpp"
#include <algorithm>
#include <vector>
#include <cassert>

namespace pot {

std::vector<SidePot> calculateSidePots(const std::vector<Player*>& players, const std::vector<int>& bets) {
    assert(players.size() == bets.size());
    std::vector<SidePot> side_pots;
    
    if (players.empty()) {
        return side_pots;
    }
    
    // Create vector of indices to sort by bet amount
    std::vector<size_t> indices(players.size());
    for (size_t i = 0; i < players.size(); ++i) {
        indices[i] = i;
    }
    
    // Sort indices by bet amount ascending
    std::sort(indices.begin(), indices.end(),
        [&bets](size_t a, size_t b) { return bets[a] < bets[b]; });
    
    // Calculate side pots
    int previous_bet = 0;
    for (size_t i = 0; i < indices.size(); ++i) {
        int current_bet = bets[indices[i]];
        if (current_bet == previous_bet) {
            continue;
        }
        
        // Players from i to end are eligible for this side pot
        SidePot pot;
        pot.amount = (current_bet - previous_bet) * (indices.size() - i);
        
        // Add eligible players
        for (size_t j = i; j < indices.size(); ++j) {
            pot.eligible_players.push_back(players[indices[j]]);
        }
        
        side_pots.push_back(pot);
        previous_bet = current_bet;
    }
    
    return side_pots;
}

std::vector<Player*> getEligiblePlayersForPot(const std::vector<Player*>& players, const std::vector<int>& bets, int pot_threshold) {
    assert(players.size() == bets.size());
    std::vector<Player*> eligible;
    
    for (size_t i = 0; i < players.size(); ++i) {
        if (bets[i] >= pot_threshold) {
            eligible.push_back(players[i]);
        }
    }
    
    return eligible;
}

void distributePot(Hand& hand, const std::vector<Player*>& winners) {
    // Distribute main pot to winners (as before)
    if (!winners.empty() && hand.pot > 0) {
        int share = hand.pot / winners.size();
        int remainder = hand.pot % winners.size();
        for (size_t i = 0; i < winners.size(); ++i) {
            int amount = share + (i < static_cast<size_t>(remainder) ? 1 : 0);
            if (amount > 0) {
                awardPot(winners[i], amount);
            }
        }
        hand.pot = 0;
    }

    // Distribute side pots
    for (SidePot& side_pot : hand.side_pots) {
        if (side_pot.amount == 0) continue;
        // Filter winners who are eligible for this side pot
        std::vector<Player*> eligible_winners;
        for (Player* winner : winners) {
            if (std::find(side_pot.eligible_players.begin(), side_pot.eligible_players.end(), winner) != side_pot.eligible_players.end()) {
                eligible_winners.push_back(winner);
            }
        }
        // If no eligible winners (should not happen), skip
        if (eligible_winners.empty()) continue;
        // Split side pot among eligible winners equally (since they already have same hand rank)
        int share = side_pot.amount / eligible_winners.size();
        int remainder = side_pot.amount % eligible_winners.size();
        for (size_t i = 0; i < eligible_winners.size(); ++i) {
            int amount = share + (i < static_cast<size_t>(remainder) ? 1 : 0);
            if (amount > 0) {
                awardPot(eligible_winners[i], amount);
            }
        }
        side_pot.amount = 0;
    }
    // Clear side pots with zero amount
    hand.side_pots.erase(std::remove_if(hand.side_pots.begin(), hand.side_pots.end(),
        [](const SidePot& sp) { return sp.amount == 0; }), hand.side_pots.end());
}

void awardPot(Player* player, int amount) {
    if (player && amount > 0) {
        player->stack += amount;
    }
}

} // namespace pot