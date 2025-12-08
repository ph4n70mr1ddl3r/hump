#pragma once

#include "connection_manager.hpp"
#include "table_manager.hpp"
#include "../core/models/player.hpp"
#include <chrono>
#include <functional>

class PlayerStateManager {
public:
    PlayerStateManager(ConnectionManager& connection_manager, TableManager& table_manager, int grace_time_ms, int removal_time_ms, std::function<void(const std::string&)> on_player_removed_callback);

    // Called when a player disconnects (websocket closed)
    void onDisconnect(Player& player);

    // Called when a player reconnects within grace period
    void onReconnect(Player& player);

    // Called when grace timer expires (player not reconnected)
    void onGraceTimerExpired(const std::string& player_id);

    // Called when removal timer expires (player inactive)
    void onRemovalTimerExpired(const std::string& player_id);

    // Check if player is considered active (connected and not sitting out)
    bool isActive(const Player& player) const;

private:
    ConnectionManager& connection_manager_;
    int grace_time_ms_;
    int removal_time_ms_;
    TableManager& table_manager_;
    std::function<void(const std::string&)> on_player_removed_callback_;

    // Helper to get current timestamp in milliseconds
    uint64_t now() const;
};
