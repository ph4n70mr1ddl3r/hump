#include <gtest/gtest.h>
#include "json_serialization.hpp"
#include <nlohmann/json.hpp>

TEST(ActionRequestMessageTest, ValidActionRequestMessageStructure) {
    // Construct an action_request message matching the contract
    nlohmann::json action_request = {
        {"type", "action_request"},
        {"payload", {
            {"hand_id", "hand123"},
            {"possible_actions", nlohmann::json::array({"fold", "call", "raise"})},
            {"call_amount", 4},
            {"min_raise", 8},
            {"max_raise", 400},
            {"timeout_ms", 30000}
        }}
    };

    // Validate required fields
    EXPECT_EQ(action_request["type"], "action_request");
    EXPECT_TRUE(action_request["payload"].is_object());

    const auto& payload = action_request["payload"];
    EXPECT_TRUE(payload["hand_id"].is_string());
    EXPECT_TRUE(payload["possible_actions"].is_array());
    EXPECT_EQ(payload["possible_actions"].size(), 3);
    EXPECT_TRUE(payload["call_amount"].is_number_integer());
    EXPECT_TRUE(payload["min_raise"].is_number_integer());
    EXPECT_TRUE(payload["max_raise"].is_number_integer());
    EXPECT_TRUE(payload["timeout_ms"].is_number_integer());
}

TEST(ActionAppliedMessageTest, ValidActionAppliedMessageStructure) {
    // Construct an action_applied message matching the contract
    nlohmann::json action_applied = {
        {"type", "action_applied"},
        {"payload", {
            {"hand_id", "hand123"},
            {"player_id", "player1"},
            {"action", "raise"},
            {"amount", 20},
            {"new_stack", 380},
            {"pot", 44},
            {"next_player_to_act", "player2"}
        }}
    };

    // Validate required fields
    EXPECT_EQ(action_applied["type"], "action_applied");
    EXPECT_TRUE(action_applied["payload"].is_object());

    const auto& payload = action_applied["payload"];
    EXPECT_TRUE(payload["hand_id"].is_string());
    EXPECT_TRUE(payload["player_id"].is_string());
    EXPECT_TRUE(payload["action"].is_string());
    EXPECT_TRUE(payload["amount"].is_number_integer());
    EXPECT_TRUE(payload["new_stack"].is_number_integer());
    EXPECT_TRUE(payload["pot"].is_number_integer());
    EXPECT_TRUE(payload["next_player_to_act"].is_string());
}

TEST(ActionRequestMessageTest, SerializationRoundTrip) {
    // TODO: implement when struct is defined
    EXPECT_TRUE(true);
}

TEST(ActionAppliedMessageTest, SerializationRoundTrip) {
    // TODO: implement when struct is defined
    EXPECT_TRUE(true);
}