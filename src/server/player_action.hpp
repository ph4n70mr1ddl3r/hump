#pragma once

#include <string>

// Forward declarations
class Hand;
class Player;

namespace player_action {

// Validate a player action (fold, call, raise) given current hand state
bool validateAction(const Hand& hand, const Player& player, const std::string& action, int amount);

// Apply a player action to the hand and update player stack, pot, etc.
bool applyAction(Hand& hand, Player& player, const std::string& action, int amount);

// Check if player can act (is active, not folded, has connection)
bool canAct(const Hand& hand, const Player& player);

// Get the minimum raise amount for current hand state
int getMinRaise(const Hand& hand);

// Get the maximum raise amount (player's stack)
int getMaxRaise(const Hand& hand, const Player& player);

} // namespace player_action