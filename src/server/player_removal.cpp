#include "player_removal.hpp"

PlayerRemovalManager::PlayerRemovalManager(TableManager& table_manager)
    : table_manager_(table_manager)
{
}

void PlayerRemovalManager::removePlayer(const std::string& player_id)
{
    table_manager_.removePlayer(player_id);
}