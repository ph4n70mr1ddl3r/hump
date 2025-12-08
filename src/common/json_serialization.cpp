#include "json_serialization.hpp"
#include <stdexcept>

// Card serialization
void to_json(nlohmann::json& j, const Card& card) {
    j = card.toString();
}

void from_json(const nlohmann::json& j, Card& card) {
    if (!j.is_string()) {
        throw std::invalid_argument("Card must be a string");
    }
    card = Card(j.get<std::string>());
}

// HandRank serialization
void to_json(nlohmann::json& j, HandRank rank) {
    j = HandRanking::rankToString(rank);
}

void from_json(const nlohmann::json& j, HandRank& rank) {
    if (!j.is_string()) {
        throw std::invalid_argument("HandRank must be a string");
    }
    std::string str = j.get<std::string>();
    // Simple mapping - could use a map
    if (str == "HIGH_CARD") rank = HandRank::HIGH_CARD;
    else if (str == "ONE_PAIR") rank = HandRank::ONE_PAIR;
    else if (str == "TWO_PAIR") rank = HandRank::TWO_PAIR;
    else if (str == "THREE_OF_A_KIND") rank = HandRank::THREE_OF_A_KIND;
    else if (str == "STRAIGHT") rank = HandRank::STRAIGHT;
    else if (str == "FLUSH") rank = HandRank::FLUSH;
    else if (str == "FULL_HOUSE") rank = HandRank::FULL_HOUSE;
    else if (str == "FOUR_OF_A_KIND") rank = HandRank::FOUR_OF_A_KIND;
    else if (str == "STRAIGHT_FLUSH") rank = HandRank::STRAIGHT_FLUSH;
    else if (str == "ROYAL_FLUSH") rank = HandRank::ROYAL_FLUSH;
    else throw std::invalid_argument("Invalid HandRank string");
}

// Vector of cards
void to_json(nlohmann::json& j, const std::vector<Card>& cards) {
    j = nlohmann::json::array();
    for (const auto& card : cards) {
        j.push_back(card);
    }
}

void from_json(const nlohmann::json& j, std::vector<Card>& cards) {
    if (!j.is_array()) {
        throw std::invalid_argument("Cards must be an array");
    }
    cards.clear();
    for (const auto& item : j) {
        cards.push_back(item.get<Card>());
    }
}

namespace json_serialization {

// WelcomeMessage serialization
void to_json(nlohmann::json& j, const WelcomeMessage& msg) {
    j = nlohmann::json::object();
    j["player_id"] = msg.player_id;
    // TODO: add table object
}

void from_json(const nlohmann::json& j, WelcomeMessage& msg) {
    if (!j.is_object()) {
        throw std::invalid_argument("WelcomeMessage must be an object");
    }
    msg.player_id = j.at("player_id").get<std::string>();
}

} // namespace json_serialization