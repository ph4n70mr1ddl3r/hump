#pragma once

#include "../card.hpp"
#include "../../common/constants.hpp"
#include <string>
#include <vector>
#include <cstdint>
#include <optional>

constexpr int TARGET_STACK = 400; // 100BB at 2/4 blinds
constexpr int TOP_UP_THRESHOLD = 20; // 5BB at 2/4 blinds

enum class ConnectionStatus {
    CONNECTED,
    DISCONNECTED,
    RECONNECTING
};

struct Player {
    std::string id; // UUID
    std::string name; // optional display name
    int stack; // current chip count
    int seat; // which seat at the table
    std::vector<Card> hole_cards; // exactly 2 cards when in a hand
    ConnectionStatus connection_status;
    uint64_t last_action_timestamp; // milliseconds since epoch
    std::optional<uint64_t> disconnected_at; // nullable timestamp
    bool is_sitting_out; // true if player has been folded due to timeout

    // Validation helper
    bool isValid() const {
        return stack >= 0 &&
               (seat == common::constants::SEAT_1 || seat == common::constants::SEAT_2) &&
               (hole_cards.size() == 0 || hole_cards.size() == 2);
    }

    // Check if player can act
    bool canAct() const {
        return connection_status == ConnectionStatus::CONNECTED &&
               !is_sitting_out;
    }

    // Check if stack is below top-up threshold (5BB = 20 chips)
    bool needsTopUp() const {
        return stack < TOP_UP_THRESHOLD;
    }

    // Top up stack to target (100BB) if below threshold
    void topUp() {
        if (needsTopUp()) {
            stack = TARGET_STACK;
        }
    }
};
