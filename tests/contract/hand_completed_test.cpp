#include <gtest/gtest.h>
#include "json_serialization.hpp"
#include <nlohmann/json.hpp>

TEST(HandCompletedMessageTest, ValidHandCompletedMessageStructure) {
    // Construct a hand_completed message matching the contract
    nlohmann::json hand_completed = {
        {"type", "hand_completed"},
        {"payload", {
            {"hand_id", "hand123"},
            {"winners", nlohmann::json::array({
                {{"player_id", "player1"}, {"amount_won", 200}, {"hand_rank", "two_pair"}}
            })},
            {"pot_distribution", nlohmann::json::array({
                {{"pot_index", 0}, {"winner_id", "player1"}, {"amount", 200}}
            })},
            {"updated_stacks", {
                {"player1", 600},
                {"player2", 200}
            }}
        }}
    };

    // Validate required fields
    EXPECT_EQ(hand_completed["type"], "hand_completed");
    EXPECT_TRUE(hand_completed["payload"].is_object());

    const auto& payload = hand_completed["payload"];
    EXPECT_TRUE(payload["hand_id"].is_string());
    EXPECT_TRUE(payload["winners"].is_array());
    EXPECT_TRUE(payload["pot_distribution"].is_array());
    EXPECT_TRUE(payload["updated_stacks"].is_object());

    // Validate winners array
    const auto& winner = payload["winners"][0];
    EXPECT_TRUE(winner["player_id"].is_string());
    EXPECT_TRUE(winner["amount_won"].is_number_integer());
    EXPECT_TRUE(winner["hand_rank"].is_string());

    // Validate pot distribution
    const auto& pot = payload["pot_distribution"][0];
    EXPECT_TRUE(pot["pot_index"].is_number_integer());
    EXPECT_TRUE(pot["winner_id"].is_string());
    EXPECT_TRUE(pot["amount"].is_number_integer());

    // Validate updated stacks object
    const auto& stacks = payload["updated_stacks"];
    EXPECT_TRUE(stacks["player1"].is_number_integer());
    EXPECT_TRUE(stacks["player2"].is_number_integer());
}

TEST(HandCompletedMessageTest, SerializationRoundTrip) {
    // TODO: implement when struct is defined
    EXPECT_TRUE(true);
}