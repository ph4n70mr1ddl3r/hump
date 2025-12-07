#include <gtest/gtest.h>
#include "hand_ranking.hpp"
#include "card.hpp"

TEST(HandRankingTest, HighCard) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Kd"), Card("Qs"), Card("Jc"), Card("9h")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::HIGH_CARD);
}

TEST(HandRankingTest, OnePair) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Ad"), Card("Qs"), Card("Jc"), Card("9h")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::ONE_PAIR);
}

TEST(HandRankingTest, TwoPair) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Ad"), Card("Qs"), Card("Qc"), Card("9h")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::TWO_PAIR);
}

TEST(HandRankingTest, ThreeOfAKind) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Ad"), Card("As"), Card("Jc"), Card("9h")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::THREE_OF_A_KIND);
}

TEST(HandRankingTest, Straight) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Kd"), Card("Qs"), Card("Jc"), Card("Th")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::STRAIGHT);
}

TEST(HandRankingTest, Flush) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Kh"), Card("Qh"), Card("Jh"), Card("9h")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::FLUSH);
}

TEST(HandRankingTest, FullHouse) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Ad"), Card("As"), Card("Jc"), Card("Jh")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::FULL_HOUSE);
}

TEST(HandRankingTest, FourOfAKind) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Ad"), Card("As"), Card("Ac"), Card("9h")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::FOUR_OF_A_KIND);
}

TEST(HandRankingTest, StraightFlush) {
    std::vector<Card> hand = {
        Card("Kh"), Card("Qh"), Card("Jh"), Card("Th"), Card("9h")
    };
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::STRAIGHT_FLUSH);
}

TEST(HandRankingTest, RoyalFlush) {
    std::vector<Card> hand = {
        Card("Ah"), Card("Kh"), Card("Qh"), Card("Jh"), Card("Th")
    };
    // Actually royal flush is ace-high straight flush
    // We'll need to differentiate in implementation
    EXPECT_EQ(HandRanking::evaluate(hand), HandRank::ROYAL_FLUSH);
}

TEST(HandRankingTest, CompareSameRank) {
    std::vector<Card> hand1 = {
        Card("Ah"), Card("Kd"), Card("Qs"), Card("Jc"), Card("9h")
    };
    std::vector<Card> hand2 = {
        Card("Kh"), Card("Qd"), Card("Js"), Card("Tc"), Card("8h")
    };
    // hand1 should be better (higher high card)
    EXPECT_GT(HandRanking::compare(hand1, hand2), 0);
}