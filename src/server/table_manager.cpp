#include "table_manager.hpp"
#include "../core/deck.hpp"
#include "../core/hand.hpp"
#include "../core/pot.hpp"
#include "player_action.hpp"
#include "../common/constants.hpp"
#include "../common/uuid.hpp"
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <chrono>

TableManager::TableManager() {
    table_.id = common::uuid::generate();
    table_.seat_1 = nullptr;
    table_.seat_2 = nullptr;
    table_.current_hand = nullptr;
    table_.pot = 0;
    table_.community_cards.clear();
    table_.dealer_button_position = common::constants::DEFAULT_DEALER_POSITION;
    table_.state = TableState::WAITING_FOR_PLAYERS;
}

bool TableManager::assignSeat(std::shared_ptr<Player> player, int seat) {
    if (seat != common::constants::SEAT_1 && seat != common::constants::SEAT_2) {
        return false;
    }
    Player** target_seat = (seat == common::constants::SEAT_1) ? &table_.seat_1 : &table_.seat_2;
    if (*target_seat != nullptr) {
        return false; // seat already occupied
    }
    // Ensure player is not already seated
    for (const auto& p : players_) {
        if (p->id == player->id) {
            return false;
        }
    }
    player->seat = seat;
    players_.push_back(player);
    *target_seat = player.get();
    return true;
}

bool TableManager::removePlayer(const std::string& player_id) {
    auto it = std::find_if(players_.begin(), players_.end(),
        [&player_id](const std::shared_ptr<Player>& p) { return p->id == player_id; });
    if (it == players_.end()) {
        return false;
    }
    Player* player = it->get();
    if (table_.seat_1 == player) {
        table_.seat_1 = nullptr;
    } else if (table_.seat_2 == player) {
        table_.seat_2 = nullptr;
    }
    players_.erase(it);
    return true;
}

std::shared_ptr<Player> TableManager::getPlayer(const std::string& player_id) const {
    auto it = std::find_if(players_.begin(), players_.end(),
        [&player_id](const std::shared_ptr<Player>& p) { return p->id == player_id; });
    return (it != players_.end()) ? *it : nullptr;
}

std::shared_ptr<Player> TableManager::findPlayer(const std::string& player_id) const {
    return getPlayer(player_id);
}

bool TableManager::startHand() {
    if (!table_.isReadyForHand()) {
        return false;
    }
    if (table_.current_hand != nullptr) {
        return false; // hand already in progress
    }

    // Create new hand
    Hand hand;
    hand.id = "hand_" + common::uuid::generate();
    hand.table = &table_;
    hand.players = {table_.seat_1, table_.seat_2};
    hand.deck = Deck(); // Default constructor creates standard deck
    hand.deck.shuffle(); // Shuffle the deck
    hand.community_cards.clear();
    hand.pot = 0;
    hand.side_pots.clear();
    hand.current_betting_round = BettingRound::PREFLOP;
    // Determine dealer position (use table_.dealer_button_position)
    // For now, set current player to act as the small blind (seat after dealer)
    hand.current_player_to_act = (table_.dealer_button_position == 0) ? table_.seat_2 : table_.seat_1;
    hand.min_raise = common::constants::BIG_BLIND;
    hand.history.clear();
    hand.winners.clear();
    hand.completed_at = 0;

    // Store hand in unique_ptr and set table reference
    this->current_hand_ = std::make_unique<Hand>(hand);
    table_.current_hand = this->current_hand_.get();

    // Deal hole cards
    for (auto player : hand.players) {
        player->hole_cards.clear();
        player->hole_cards.push_back(hand.deck.deal());
        player->hole_cards.push_back(hand.deck.deal());
    }

    // Update table state
    table_.state = TableState::HAND_IN_PROGRESS;
    table_.community_cards.clear();
    table_.pot = 0;

    return true;
}

void TableManager::endHand() {
    if (table_.current_hand == nullptr) {
        return;
    }
    Hand* hand = table_.current_hand;

    // Determine winners
    std::vector<Player*> winners = poker::determineWinners(*hand);
    hand->winners = winners;

    // Distribute pot (main pot and side pots)
    pot::distributePot(*hand, winners);

    // Top up players if needed (between hands)
    for (auto& player : players_) {
        player->topUp();
    }

    // Update table pot (should be zero after distribution)
    if (hand->pot != 0) {
        common::log::log(common::log::Level::WARN, "Pot not fully distributed after hand end: " + std::to_string(hand->pot));
    }
    table_.pot = 0;

    // Set completion timestamp
    hand->completed_at = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Clear current hand
    current_hand_.reset();
    table_.current_hand = nullptr;
    table_.state = TableState::WAITING_FOR_PLAYERS;
    table_.community_cards.clear();
    // Rotate dealer button
    table_.dealer_button_position = (table_.dealer_button_position + 1) % (common::constants::SEAT_2 + 1);
}

bool TableManager::processPlayerAction(const std::string& player_id, const std::string& action, int amount) {
    Hand* hand = table_.current_hand;
    if (!hand) return false;

    auto player = findPlayer(player_id);
    if (!player) return false;

    // Validate and apply action using player_action module
    if (!player_action::validateAction(*hand, *player, action, amount))
    {
        return false;
    }

    if (!player_action::applyAction(*hand, *player, action, amount))
    {
        return false;
    }

    // Update table pot from hand pot
    table_.pot = hand->pot;

    return true;
}

void TableManager::dealHoleCards() {
    // Already done in startHand
}

void TableManager::dealCommunityCards() {
    if (!table_.current_hand) {
        return;
    }

    Hand* hand = table_.current_hand;
    int count = 0;

    switch (hand->current_betting_round) {
        case BettingRound::PREFLOP:
            count = 0;
            break;
        case BettingRound::FLOP:
            count = 3;
            break;
        case BettingRound::TURN:
        case BettingRound::RIVER:
            count = 1;
            break;
        case BettingRound::SHOWDOWN:
            count = 0;
            break;
    }

    for (int i = 0; i < count; ++i) {
        if (hand->deck.size() == 0) {
            common::log::log(common::log::Level::ERROR, "Deck exhausted while dealing community cards");
            return;
        }
        Card card = hand->deck.deal();
        hand->community_cards.push_back(card);
        table_.community_cards.push_back(card);
    }
}

void TableManager::advanceBettingRound() {
    if (!table_.current_hand) {
        return;
    }

    Hand* hand = table_.current_hand;
    bool advanced = hand->advanceRound();

    if (advanced) {
        dealCommunityCards();
    }
}