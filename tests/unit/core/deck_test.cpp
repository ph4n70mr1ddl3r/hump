#include <gtest/gtest.h>
#include "card.hpp"
#include "deck.hpp"

TEST(CardTest, ConstructorFromString) {
    Card card("Ah");
    EXPECT_EQ(card.rank(), Rank::ACE);
    EXPECT_EQ(card.suit(), Suit::HEARTS);
    EXPECT_EQ(card.toString(), "Ah");
}

TEST(CardTest, ConstructorFromRankSuit) {
    Card card(Rank::KING, Suit::SPADES);
    EXPECT_EQ(card.toString(), "Ks");
}

TEST(CardTest, Equality) {
    Card c1("Ah");
    Card c2("Ah");
    Card c3("Kd");
    EXPECT_EQ(c1, c2);
    EXPECT_NE(c1, c3);
}

TEST(DeckTest, Initialization) {
    Deck deck;
    EXPECT_EQ(deck.size(), 52);
}

TEST(DeckTest, ShuffleChangesOrder) {
    Deck deck1;
    Deck deck2;
    deck2.shuffle();
    
    // Probability of identical order after shuffle is 1/52! ≈ 0
    // We'll just check that at least one card is in different position
    bool different = false;
    for (int i = 0; i < 52; ++i) {
        if (deck1.deal() != deck2.deal()) {
            different = true;
            break;
        }
    }
    EXPECT_TRUE(different);
}

TEST(DeckTest, DealReducesSize) {
    Deck deck;
    EXPECT_EQ(deck.size(), 52);
    deck.deal();
    EXPECT_EQ(deck.size(), 51);
}

TEST(DeckTest, DealAllCards) {
    Deck deck;
    for (int i = 0; i < 52; ++i) {
        EXPECT_NO_THROW(deck.deal());
    }
    EXPECT_EQ(deck.size(), 0);
    EXPECT_THROW(deck.deal(), std::out_of_range);
}