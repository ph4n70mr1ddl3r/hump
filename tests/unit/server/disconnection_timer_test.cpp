#include <gtest/gtest.h>
#include "../../src/server/connection_manager.hpp"
#include <boost/asio.hpp>
#include <atomic>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

class DisconnectionTimerTest : public ::testing::Test {
protected:
    boost::asio::io_context ioc;
    ConnectionManager cm{ioc};
    
    void runIoContextFor(int ms) {
        ioc.restart();
        auto start = std::chrono::steady_clock::now();
        while (ioc.run_one() > 0) {
            if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(ms)) {
                break;
            }
        }
    }
};

TEST_F(DisconnectionTimerTest, GraceTimerFires) {
    std::atomic<bool> fired{false};
    std::string player_id = "player1";
    
    cm.startGraceTimer(player_id, 50, [&fired](const std::string& pid) {
        EXPECT_EQ(pid, "player1");
        fired = true;
    });
    
    // Run io_context to allow timer to expire
    runIoContextFor(100);
    
    EXPECT_TRUE(fired);
}

TEST_F(DisconnectionTimerTest, GraceTimerCancelled) {
    std::atomic<bool> fired{false};
    std::string player_id = "player2";
    
    cm.startGraceTimer(player_id, 100, [&fired](const std::string&) {
        fired = true;
    });
    
    // Cancel before expiry
    cm.cancelTimers(player_id);
    
    runIoContextFor(150);
    
    EXPECT_FALSE(fired);
    EXPECT_FALSE(cm.hasActiveTimers(player_id));
}

TEST_F(DisconnectionTimerTest, RemovalTimerFiresAfterGrace) {
    std::atomic<int> fire_count{0};
    std::string player_id = "player3";
    
    // Start grace timer (short) and removal timer (long)
    cm.startGraceTimer(player_id, 30, [&fire_count](const std::string&) {
        fire_count++;
    });
    cm.startRemovalTimer(player_id, 60, [&fire_count](const std::string&) {
        fire_count++;
    });
    
    runIoContextFor(200);
    
    // Both timers should have fired
    EXPECT_EQ(fire_count, 2);
    EXPECT_FALSE(cm.hasActiveTimers(player_id));
}

TEST_F(DisconnectionTimerTest, HasActiveTimers) {
    std::string player_id = "player4";
    EXPECT_FALSE(cm.hasActiveTimers(player_id));
    
    cm.startGraceTimer(player_id, 500, [](const std::string&) {});
    EXPECT_TRUE(cm.hasActiveTimers(player_id));
    
    cm.cancelTimers(player_id);
    EXPECT_FALSE(cm.hasActiveTimers(player_id));
}