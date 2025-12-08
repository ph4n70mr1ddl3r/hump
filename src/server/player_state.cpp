#include "player_state.hpp"
#include <chrono>

PlayerStateManager::PlayerStateManager(ConnectionManager& connection_manager, TableManager& table_manager, int grace_time_ms, int removal_time_ms, std::function<void(const std::string&)> on_player_removed_callback)
    : connection_manager_(connection_manager),
      table_manager_(table_manager),
      grace_time_ms_(grace_time_ms),
      removal_time_ms_(removal_time_ms),
      on_player_removed_callback_(on_player_removed_callback)
{
}

void PlayerStateManager::onDisconnect(Player& player)
{
    player.connection_status = ConnectionStatus::DISCONNECTED;
    player.disconnected_at = now();
    player.is_sitting_out = false; // not yet folded

    // Start grace timer
    connection_manager_.startGraceTimer(player.id, grace_time_ms_,
        [this](const std::string& player_id) {
            onGraceTimerExpired(player_id);
        });
}

void PlayerStateManager::onReconnect(Player& player)
{
    // Cancel any active timers
    connection_manager_.cancelTimers(player.id);

    // Update player state
    player.connection_status = ConnectionStatus::CONNECTED;
    player.disconnected_at = std::nullopt;
    player.is_sitting_out = false;
}

void PlayerStateManager::onGraceTimerExpired(const std::string& player_id)
{
    // Mark player as sitting out (folded)
    auto player = table_manager_.getPlayer(player_id);
    if (player)
    {
        player->is_sitting_out = true;
    }
    // Start removal timer
    connection_manager_.startRemovalTimer(player_id, removal_time_ms_,
        [this](const std::string& player_id) {
            onRemovalTimerExpired(player_id);
        });
}

void PlayerStateManager::onRemovalTimerExpired(const std::string& player_id)
{
    // Remove player from table
    table_manager_.removePlayer(player_id);
    // Notify game session to broadcast player_removed
    if (on_player_removed_callback_)
    {
        on_player_removed_callback_(player_id);
    }
}

bool PlayerStateManager::isActive(const Player& player) const
{
    return player.connection_status == ConnectionStatus::CONNECTED && !player.is_sitting_out;
}

uint64_t PlayerStateManager::now() const
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}