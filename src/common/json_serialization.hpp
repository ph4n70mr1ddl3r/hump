#pragma once

#include <nlohmann/json.hpp>
#include "../core/card.hpp"
#include "../core/hand_ranking.hpp"
#include <vector>

// Card serialization
void to_json(nlohmann::json& j, const Card& card);
void from_json(const nlohmann::json& j, Card& card);

// HandRank serialization
void to_json(nlohmann::json& j, HandRank rank);
void from_json(const nlohmann::json& j, HandRank& rank);

// Vector of cards
void to_json(nlohmann::json& j, const std::vector<Card>& cards);
void from_json(const nlohmann::json& j, std::vector<Card>& cards);

namespace json_serialization {

// WebSocket message types (from contracts)
struct WelcomeMessage {
    std::string player_id;
    // ... other fields
};

void to_json(nlohmann::json& j, const WelcomeMessage& msg);
void from_json(const nlohmann::json& j, WelcomeMessage& msg);

// Helper to serialize any type to string
template<typename T>
std::string serialize(const T& obj) {
    nlohmann::json j;
    to_json(j, obj);
    return j.dump();
}

// Helper to deserialize string to type
template<typename T>
T deserialize(const std::string& str) {
    nlohmann::json j = nlohmann::json::parse(str);
    T obj;
    from_json(j, obj);
    return obj;
}

} // namespace json_serialization
