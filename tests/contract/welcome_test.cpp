#include <gtest/gtest.h>
#include "json_serialization.hpp"
#include <nlohmann/json.hpp>

TEST(WelcomeMessageTest, ValidWelcomeMessageStructure) {
    // Construct a welcome message matching the contract
    nlohmann::json welcome = {
        {"type", "welcome"},
        {"payload", {
            {"player_id", "player123"},
            {"table", {
                {"seat_1", {{"player_id", "seat1_uuid"}, {"stack", 400}, {"connected", true}}},
                {"seat_2", {{"player_id", "seat2_uuid"}, {"stack", 400}, {"connected", true}}},
                {"current_hand", nullptr},
                {"pot", 0},
                {"community_cards", nlohmann::json::array()},
                {"dealer_button_position", 0}
            }}
        }}
    };

    // Validate required fields
    EXPECT_EQ(welcome["type"], "welcome");
    EXPECT_TRUE(welcome["payload"].is_object());
    EXPECT_TRUE(welcome["payload"]["player_id"].is_string());
    EXPECT_TRUE(welcome["payload"]["table"].is_object());

    // Validate table structure
    const auto& table = welcome["payload"]["table"];
    EXPECT_TRUE(table["seat_1"].is_object());
    EXPECT_TRUE(table["seat_2"].is_object());
    EXPECT_TRUE(table["current_hand"].is_null());
    EXPECT_TRUE(table["pot"].is_number_integer());
    EXPECT_TRUE(table["community_cards"].is_array());
    EXPECT_TRUE(table["dealer_button_position"].is_number_integer());
    EXPECT_EQ(table["dealer_button_position"].get<int>(), 0);

    // Validate seat objects
    const auto& seat1 = table["seat_1"];
    EXPECT_TRUE(seat1["player_id"].is_string());
    EXPECT_TRUE(seat1["stack"].is_number_integer());
    EXPECT_TRUE(seat1["connected"].is_boolean());

    const auto& seat2 = table["seat_2"];
    EXPECT_TRUE(seat2["player_id"].is_string());
    EXPECT_TRUE(seat2["stack"].is_number_integer());
    EXPECT_TRUE(seat2["connected"].is_boolean());
}

TEST(WelcomeMessageTest, SerializationRoundTrip) {
    // Create WelcomeMessage object (requires struct definition)
    json_serialization::WelcomeMessage msg;
    msg.player_id = "test_player";

    // Serialize
    nlohmann::json j = msg;
    EXPECT_EQ(j["player_id"], "test_player");

    // Deserialize
    json_serialization::WelcomeMessage msg2 = j.get<json_serialization::WelcomeMessage>();
    EXPECT_EQ(msg.player_id, msg2.player_id);
}