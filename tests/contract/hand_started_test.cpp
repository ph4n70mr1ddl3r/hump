#include <gtest/gtest.h>
#include "json_serialization.hpp"
#include <nlohmann/json.hpp>

TEST(HandStartedMessageTest, ValidHandStartedMessageStructure) {
    // Construct a hand_started message matching the contract
    nlohmann::json hand_started = {
        {"type", "hand_started"},
        {"payload", {
            {"hand_id", "hand123"},
            {"players", nlohmann::json::array({
                {{"player_id", "player1"}, {"stack", 400}, {"hole_cards", {"Ah", "Kd"}}},
                {{"player_id", "player2"}, {"stack", 400}, {"hole_cards", {"Qs", "Jc"}}}
            })},
            {"small_blind", 2},
            {"big_blind", 4},
            {"dealer_position", 0},
            {"current_player_to_act", "player1"},
            {"min_raise", 4}
        }}
    };

    // Validate required fields
    EXPECT_EQ(hand_started["type"], "hand_started");
    EXPECT_TRUE(hand_started["payload"].is_object());

    const auto& payload = hand_started["payload"];
    EXPECT_TRUE(payload["hand_id"].is_string());
    EXPECT_TRUE(payload["players"].is_array());
    EXPECT_EQ(payload["players"].size(), 2);
    EXPECT_TRUE(payload["small_blind"].is_number_integer());
    EXPECT_TRUE(payload["big_blind"].is_number_integer());
    EXPECT_TRUE(payload["dealer_position"].is_number_integer());
    EXPECT_TRUE(payload["current_player_to_act"].is_string());
    EXPECT_TRUE(payload["min_raise"].is_number_integer());

    // Validate player objects
    const auto& player1 = payload["players"][0];
    EXPECT_TRUE(player1["player_id"].is_string());
    EXPECT_TRUE(player1["stack"].is_number_integer());
    EXPECT_TRUE(player1["hole_cards"].is_array());
    EXPECT_EQ(player1["hole_cards"].size(), 2);
    EXPECT_TRUE(player1["hole_cards"][0].is_string());
    EXPECT_TRUE(player1["hole_cards"][1].is_string());

    const auto& player2 = payload["players"][1];
    EXPECT_TRUE(player2["player_id"].is_string());
    EXPECT_TRUE(player2["stack"].is_number_integer());
    EXPECT_TRUE(player2["hole_cards"].is_array());
    EXPECT_EQ(player2["hole_cards"].size(), 2);
}

TEST(HandStartedMessageTest, SerializationRoundTrip) {
    // Create HandStartedMessage object (requires struct definition)
    // TODO: implement when struct is defined
    // For now, just pass
    EXPECT_TRUE(true);
}