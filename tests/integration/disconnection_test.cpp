#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include "../../src/server/connection_manager.hpp"
#include "../../src/core/models/player.hpp"
#include <thread>
#include <chrono>
#include <atomic>

namespace asio = boost::asio;

TEST(DisconnectionIntegrationTest, GraceTimerStartsOnDisconnection) {
    asio::io_context ioc;
    ConnectionManager cm(ioc);
    
    // Create a player
    Player player;
    player.id = "player1";
    player.connection_status = ConnectionStatus::CONNECTED;
    player.is_sitting_out = false;
    
    // Simulate disconnection: start grace timer
    std::atomic<bool> grace_fired{false};
    cm.startGraceTimer(player.id, 50, [&grace_fired](const std::string& pid) {
        EXPECT_EQ(pid, "player1");
        grace_fired = true;
    });
    
    // Run io_context to allow timer to expire
    ioc.restart();
    auto start = std::chrono::steady_clock::now();
    while (ioc.run_one() > 0) {
        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(200)) {
            break;
        }
    }
    
    EXPECT_TRUE(grace_fired);
}

TEST(DisconnectionIntegrationTest, ReconnectionCancelsGraceTimer) {
    asio::io_context ioc;
    ConnectionManager cm(ioc);
    
    Player player;
    player.id = "player2";
    player.connection_status = ConnectionStatus::DISCONNECTED;
    
    std::atomic<bool> grace_fired{false};
    cm.startGraceTimer(player.id, 100, [&grace_fired](const std::string&) {
        grace_fired = true;
    });
    
    // Simulate reconnection before grace expires
    cm.cancelTimers(player.id);
    
    ioc.restart();
    auto start = std::chrono::steady_clock::now();
    while (ioc.run_one() > 0) {
        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(150)) {
            break;
        }
    }
    
    EXPECT_FALSE(grace_fired);
    EXPECT_FALSE(cm.hasActiveTimers(player.id));
}

TEST(DisconnectionIntegrationTest, RemovalTimerFiresAfterGrace) {
    asio::io_context ioc;
    ConnectionManager cm(ioc);
    
    Player player;
    player.id = "player3";
    player.connection_status = ConnectionStatus::DISCONNECTED;
    
    std::atomic<int> timer_fired{0};
    // Start grace timer (short) and removal timer (long)
    cm.startGraceTimer(player.id, 30, [&timer_fired](const std::string&) {
        timer_fired++;
    });
    cm.startRemovalTimer(player.id, 60, [&timer_fired](const std::string&) {
        timer_fired++;
    });
    
    ioc.restart();
    auto start = std::chrono::steady_clock::now();
    while (ioc.run_one() > 0) {
        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(200)) {
            break;
        }
    }
    
    // Both timers should have fired
    EXPECT_EQ(timer_fired, 2);
    EXPECT_FALSE(cm.hasActiveTimers(player.id));
}