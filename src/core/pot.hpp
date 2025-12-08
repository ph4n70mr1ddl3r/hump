#pragma once

#include "models/table.hpp"
#include "models/hand.hpp"
#include <vector>

namespace pot {

// Calculate side pots based on player bets and all-in situations
std::vector<SidePot> calculateSidePots(const std::vector<Player*>& players, const std::vector<int>& bets);

// Distribute pot to winners based on hand ranking
void distributePot(Hand& hand, const std::vector<Player*>& winners);

// Update player stacks after pot distribution
void awardPot(Player* player, int amount);

// Determine which players are eligible for each side pot
std::vector<Player*> getEligiblePlayersForPot(const std::vector<Player*>& players, const std::vector<int>& bets, int pot_threshold);

} // namespace pot
