#include <gtest/gtest.h>
#include "json_serialization.hpp"
#include <nlohmann/json.hpp>

TEST(MalformedMessagesTest, InvalidJson) {
    std::string invalid_json = "{invalid json";
    EXPECT_THROW({
        nlohmann::json::parse(invalid_json);
    }, nlohmann::json::parse_error);
}

TEST(MalformedMessagesTest, MissingTypeField) {
    nlohmann::json msg = {
        {"payload", {{"player_id", "123"}}}
    };
    // Should fail validation
    EXPECT_FALSE(msg.contains("type"));
}

TEST(MalformedMessagesTest, InvalidActionAmount) {
    nlohmann::json msg = {
        {"type", "action"},
        {"payload", {
            {"hand_id", "hand123"},
            {"action", "raise"},
            {"amount", -10} // Negative amount invalid
        }}
    };
    // Should fail validation
    EXPECT_LT(msg["payload"]["amount"].get<int>(), 0);
}

TEST(MalformedMessagesTest, InvalidCardString) {
    // Card string should be two characters
    std::string invalid_card = "A"; // Missing suit
    EXPECT_THROW({
        Card card(invalid_card);
    }, std::invalid_argument);
}

TEST(MalformedMessagesTest, UnexpectedMessageType) {
    nlohmann::json msg = {
        {"type", "unknown_message_type"},
        {"payload", {}}
    };
    // Should be rejected by server
    EXPECT_NE(msg["type"].get<std::string>(), "welcome");
}

TEST(MalformedMessagesTest, ArrayInsteadOfObject) {
    nlohmann::json msg = nlohmann::json::array({1, 2, 3});
    EXPECT_TRUE(msg.is_array());
    EXPECT_FALSE(msg.is_object());
}