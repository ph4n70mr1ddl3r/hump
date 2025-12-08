#pragma once

#include "../core/models/table.hpp"
#include "../core/models/player.hpp"
#include "../core/models/hand.hpp"
#include <memory>
#include <optional>
#include <vector>

class TableManager {
public:
    TableManager();

    // Seat management
    bool assignSeat(std::shared_ptr<Player> player, int seat);
    bool removePlayer(const std::string& player_id);
    std::shared_ptr<Player> getPlayer(const std::string& player_id) const;

    // Table state
    const Table& getTable() const { return table_; }
    TableState getTableState() const { return table_.state; }
    bool isReadyForHand() const { return table_.isReadyForHand(); }

    // Hand management
    bool startHand();
    void endHand();
    const Hand* getCurrentHand() const { return table_.current_hand; }

    // Player actions (to be implemented in player_action.cpp)
    bool processPlayerAction(const std::string& player_id, const std::string& action, int amount);

private:
    Table table_;
    std::vector<std::shared_ptr<Player>> players_;
    std::unique_ptr<Hand> current_hand_;

    void dealHoleCards();
    void dealCommunityCards();
    void advanceBettingRound();

    // Helper to find player by ID
    std::shared_ptr<Player> findPlayer(const std::string& player_id) const;
};
