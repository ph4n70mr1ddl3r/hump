#include "connection_manager.hpp"

ConnectionManager::ConnectionManager(boost::asio::io_context& ioc)
    : ioc_(ioc)
{
}

void ConnectionManager::startGraceTimer(const std::string& player_id, int grace_time_ms, TimerCallback on_expiry)
{
    auto& timers = timers_[player_id];
    if (!timers.grace_timer)
    {
        timers.grace_timer = std::make_unique<boost::asio::steady_timer>(ioc_);
    }
    else
    {
        timers.grace_timer->cancel();
    }

    timers.grace_timer->expires_after(std::chrono::milliseconds(grace_time_ms));
    timers.grace_timer->async_wait([this, player_id, on_expiry](const boost::system::error_code& ec) {
        if (!ec)
        {
            on_expiry(player_id);
            // After grace timer expires, we can remove the timer entry
            auto it = timers_.find(player_id);
            if (it != timers_.end())
            {
                it->second.grace_timer.reset();
                // If both timers are null, remove entry safely
                if (!it->second.removal_timer)
                {
                    timers_.erase(it);
                }
            }
        }
    });
}

void ConnectionManager::startRemovalTimer(const std::string& player_id, int removal_time_ms, TimerCallback on_expiry)
{
    auto& timers = timers_[player_id];
    if (!timers.removal_timer)
    {
        timers.removal_timer = std::make_unique<boost::asio::steady_timer>(ioc_);
    }
    else
    {
        timers.removal_timer->cancel();
    }

    timers.removal_timer->expires_after(std::chrono::milliseconds(removal_time_ms));
    timers.removal_timer->async_wait([this, player_id, on_expiry](const boost::system::error_code& ec) {
        if (!ec)
        {
            on_expiry(player_id);
            // After removal timer expires, remove entry
            timers_.erase(player_id);
        }
    });
}

void ConnectionManager::cancelTimers(const std::string& player_id)
{
    auto it = timers_.find(player_id);
    if (it != timers_.end())
    {
        if (it->second.grace_timer)
        {
            it->second.grace_timer->cancel();
        }
        if (it->second.removal_timer)
        {
            it->second.removal_timer->cancel();
        }
        timers_.erase(it);
    }
}

bool ConnectionManager::hasActiveTimers(const std::string& player_id) const
{
    auto it = timers_.find(player_id);
    if (it == timers_.end())
    {
        return false;
    }
    return it->second.grace_timer || it->second.removal_timer;
}