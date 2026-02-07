#pragma once

#include "models/hand.hpp"
#include "deck.hpp"
#include <vector>
#include <string>

namespace poker {

// Initialize a new hand: shuffle deck, assign dealer, set blinds, deal hole cards
void startHand(Hand& hand, Deck& deck, Player* dealer, Player* small_blind, Player* big_blind);

// Deal hole cards to each player in the hand
void dealHoleCards(Hand& hand, Deck& deck);

// Deal community cards (flop: 3, turn: 1, river: 1)
void dealCommunityCards(Hand& hand, Deck& deck, int count);

// Apply a player action (fold, call, raise) and update hand state
bool applyAction(Hand& hand, Player* player, const std::string& action, int amount);

// Advance to the next betting round (preflop -> flop -> turn -> river -> showdown)
bool advanceBettingRound(Hand& hand);

// Check if the hand is complete (only one player remaining or showdown reached)
bool isHandComplete(const Hand& hand);

// Determine winner(s) of the hand (multiple players can split pot)
std::vector<Player*> determineWinners(const Hand& hand);

// Calculate side pots based on all-in situations
void calculateSidePots(Hand& hand);

// Reset hand for a new hand (clear cards, pots, etc.)
void resetHand(Hand& hand);

} // namespace poker
