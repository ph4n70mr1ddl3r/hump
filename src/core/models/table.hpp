#pragma once

#include "../card.hpp"
#include <cstdint>
#include <vector>
#include <optional>
#include <string>

// Forward declarations
class Player;
class Hand;

enum class TableState {
    WAITING_FOR_PLAYERS,
    HAND_IN_PROGRESS,
    HAND_COMPLETE
};

struct SidePot {
    int amount;
    std::vector<Player*> eligible_players;
};

struct Table {
    std::string id; // UUID
    Player* seat_1; // reference to Player in seat 1 (button)
    Player* seat_2; // reference to Player in seat 2 (big blind)
    Hand* current_hand; // nullable
    int pot; // total chips in main pot
    std::vector<SidePot> side_pots;
    std::vector<Card> community_cards; // 0-5 cards
    int dealer_button_position; // 0 or 1
    TableState state;

    // Validation helper
    bool isValid() const {
        return (seat_1 != nullptr && seat_2 != nullptr) &&
               (community_cards.size() <= 5) &&
               (dealer_button_position == 0 || dealer_button_position == 1);
    }

    // Check if table is ready to start a hand
    bool isReadyForHand() const {
        return state == TableState::WAITING_FOR_PLAYERS &&
               seat_1 != nullptr && seat_2 != nullptr;
    }
};
