#pragma once

#include <string>

// Forward declarations
class Hand;
class Player;

namespace player_action {

[[nodiscard]] bool validateAction(const Hand& hand, const Player& player, const std::string& action, int amount);

bool applyAction(Hand& hand, Player& player, const std::string& action, int amount);

[[nodiscard]] bool canAct(const Hand& hand, const Player& player);

[[nodiscard]] int getMinRaise(const Hand& hand);

[[nodiscard]] int getMaxRaise(const Hand& hand, const Player& player);

} // namespace player_action
