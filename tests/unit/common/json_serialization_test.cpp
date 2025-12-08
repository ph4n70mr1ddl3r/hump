#include <gtest/gtest.h>
#include "json_serialization.hpp"

TEST(JsonSerializationTest, CardSerialization) {
    Card card("Ah");
    nlohmann::json j = card;
    EXPECT_EQ(j, "Ah");
    
    Card card2 = j.get<Card>();
    EXPECT_EQ(card, card2);
}

TEST(JsonSerializationTest, HandRankSerialization) {
    nlohmann::json j = HandRank::STRAIGHT_FLUSH;
    EXPECT_EQ(j, "STRAIGHT_FLUSH");
    
    HandRank rank = j.get<HandRank>();
    EXPECT_EQ(rank, HandRank::STRAIGHT_FLUSH);
}

TEST(JsonSerializationTest, VectorOfCards) {
    std::vector<Card> cards = {Card("Ah"), Card("Kd"), Card("Qs")};
    nlohmann::json j = cards;
    EXPECT_EQ(j.size(), 3);
    EXPECT_EQ(j[0], "Ah");
    
    std::vector<Card> cards2 = j.get<std::vector<Card>>();
    EXPECT_EQ(cards, cards2);
}

TEST(JsonSerializationTest, WelcomeMessage) {
    json_serialization::WelcomeMessage msg;
    msg.player_id = "player123";
    
    nlohmann::json j = msg;
    EXPECT_EQ(j["player_id"], "player123");
    
    json_serialization::WelcomeMessage msg2 = j.get<json_serialization::WelcomeMessage>();
    EXPECT_EQ(msg.player_id, msg2.player_id);
}

TEST(JsonSerializationTest, SerializeDeserializeHelpers) {
    Card card("Th");
    std::string str = json_serialization::serialize(card);
    Card card2 = json_serialization::deserialize<Card>(str);
    EXPECT_EQ(card, card2);
}