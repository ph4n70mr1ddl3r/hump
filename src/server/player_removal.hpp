#pragma once

#include "table_manager.hpp"
#include <string>

class PlayerRemovalManager {
public:
    explicit PlayerRemovalManager(TableManager& table_manager);
    
    // Remove player after removal timeout expires
    void removePlayer(const std::string& player_id);
    
private:
    TableManager& table_manager_;
};