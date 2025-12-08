#pragma once

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

class ConnectionManager {
public:
    using TimerCallback = std::function<void(const std::string& player_id)>;

    ConnectionManager(boost::asio::io_context& ioc);

    // Start grace timer for disconnected player
    void startGraceTimer(const std::string& player_id, int grace_time_ms, TimerCallback on_expiry);

    // Start removal timer for inactive player (after grace period)
    void startRemovalTimer(const std::string& player_id, int removal_time_ms, TimerCallback on_expiry);

    // Cancel timers for a player (if reconnected)
    void cancelTimers(const std::string& player_id);

    // Check if player has active timers
    bool hasActiveTimers(const std::string& player_id) const;

private:
    boost::asio::io_context& ioc_;

    struct PlayerTimers {
        std::unique_ptr<boost::asio::steady_timer> grace_timer;
        std::unique_ptr<boost::asio::steady_timer> removal_timer;
    };

    std::unordered_map<std::string, PlayerTimers> timers_;
};
