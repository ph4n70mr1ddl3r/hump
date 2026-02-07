#include "game_session.hpp"
#include "../common/json_serialization.hpp"
#include "../common/uuid.hpp"
#include "../common/constants.hpp"
#include "../common/logging.hpp"
#include "../core/hand.hpp"
#include <boost/beast.hpp>
#include <iostream>

namespace beast = boost::beast;

GameSession::GameSession(boost::asio::io_context& ioc, int action_timeout_ms, int disconnect_grace_time_ms, int removal_timeout_ms)
    : action_timeout_ms_(action_timeout_ms),
      disconnect_grace_time_ms_(disconnect_grace_time_ms),
      removal_timeout_ms_(removal_timeout_ms),
      ioc_(ioc),
      connection_manager_(ioc),
      player_state_manager_(connection_manager_, table_manager_, disconnect_grace_time_ms, removal_timeout_ms,
          [self = shared_from_this()](const std::string& player_id) {
              if (self) {
                  self->broadcastPlayerRemoved(player_id);
              }
          })
{
}

nlohmann::json GameSession::createErrorResponse(const std::string& code, const std::string& message) const
{
    return {
        {"type", "error"},
        {"payload", {
            {"code", code},
            {"message", message}
        }}
    };
}

void GameSession::handleMessage(const std::string& message, std::shared_ptr<WebSocketSession> session)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "handleMessage: null session");
        return;
    }

    try
    {
        nlohmann::json json = nlohmann::json::parse(message);
        if (!json.contains("type")) {
            sendJson(session, createErrorResponse("invalid_json", "Missing 'type' field"));
            return;
        }
        if (!json.contains("payload")) {
            sendJson(session, createErrorResponse("invalid_json", "Missing 'payload' field"));
            return;
        }
        std::string type = json.at("type").get<std::string>();

        if (type == "join")
        {
            handleJoin(json.at("payload"), session);
        }
        else if (type == "action")
        {
            handleAction(json.at("payload"), session);
        }
        else if (type == "ping")
        {
            handlePing(json.at("payload"), session);
        }
        else if (type == "top_up")
        {
            handleTopUp(json.at("payload"), session);
        }
        else
        {
            sendJson(session, createErrorResponse("invalid_message_type", "Unknown message type"));
        }
    }
    catch (const nlohmann::json::exception& e)
    {
        sendJson(session, createErrorResponse("invalid_json", "Failed to parse JSON"));
    }
    catch (const std::exception& e)
    {
        common::log::log(common::log::Level::ERROR, "handleMessage exception: " + std::string(e.what()));
        sendJson(session, createErrorResponse("internal_error", "Internal server error"));
    }
}

void GameSession::sendWelcome(std::shared_ptr<WebSocketSession> session)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "sendWelcome: null session");
        return;
    }

    std::string player_id = generatePlayerId();
    registerSession(player_id, session);

    common::log::log(common::log::Level::INFO, "Welcome sent to player_id: " + player_id);

    nlohmann::json welcome = {
        {"type", "welcome"},
        {"payload", {
            {"player_id", player_id},
            {"table", {
                {"seat_1", nullptr},
                {"seat_2", nullptr},
                {"current_hand", nullptr},
                {"pot", 0},
                {"community_cards", nlohmann::json::array()},
                {"dealer_button_position", 0}
            }}
        }}
    };
    sendJson(session, welcome);
}

void GameSession::broadcastHandStarted()
{
    const Hand* hand = table_manager_.getCurrentHand();
    if (!hand) return;

    const Table& table = table_manager_.getTable();

    nlohmann::json players_array = nlohmann::json::array();
    for (Player* player : hand->players)
    {
        if (!player) continue;
        nlohmann::json player_json;
        player_json["player_id"] = player->id;
        player_json["stack"] = player->stack;
        // Include hole cards (convert Card to string)
        nlohmann::json cards_json = nlohmann::json::array();
        for (const Card& card : player->hole_cards)
        {
            cards_json.push_back(card.toString());
        }
        player_json["hole_cards"] = cards_json;
        players_array.push_back(player_json);
    }

    // Determine small blind and big blind amounts (fixed per spec)
    int small_blind = common::constants::SMALL_BLIND;
    int big_blind = common::constants::BIG_BLIND;

    // Determine current player to act ID
    std::string current_player_id = "";
    if (hand->current_player_to_act)
    {
        current_player_id = hand->current_player_to_act->id;
    }

    nlohmann::json payload = {
        {"hand_id", hand->id},
        {"players", players_array},
        {"small_blind", small_blind},
        {"big_blind", big_blind},
        {"dealer_position", table.dealer_button_position},
        {"current_player_to_act", current_player_id},
        {"min_raise", hand->min_raise}
    };

    nlohmann::json message = {
        {"type", "hand_started"},
        {"payload", payload}
    };

    broadcastJson(message);
}

void GameSession::sendActionRequest(const std::string& player_id)
{
    const Hand* hand = table_manager_.getCurrentHand();
    if (!hand) return;

    auto player = table_manager_.getPlayer(player_id);
    if (!player) return;

    // Determine possible actions (simplified)
    nlohmann::json possible_actions = nlohmann::json::array({"fold", "call", "raise"});

    // Calculate call amount (amount needed to call current bet)
    // In a real implementation, this would be based on the highest bet in the current round
    int call_amount = common::constants::BIG_BLIND;

    // Get min raise from hand
    int min_raise = hand->min_raise;

    // Max raise is player's stack
    int max_raise = player->stack;

    // Timeout (configurable, default 30 seconds)
    int timeout_ms = common::constants::ACTION_TIMEOUT_MS;

    nlohmann::json payload = {
        {"hand_id", hand->id},
        {"possible_actions", possible_actions},
        {"call_amount", call_amount},
        {"min_raise", min_raise},
        {"max_raise", max_raise},
        {"timeout_ms", timeout_ms}
    };

    nlohmann::json message = {
        {"type", "action_request"},
        {"payload", payload}
    };

    // Send to specific player
    std::shared_ptr<WebSocketSession> session;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto session_it = player_sessions_.find(player_id);
        if (session_it != player_sessions_.end())
        {
            session = session_it->second;
        }
    }
    if (session)
    {
        sendJson(session, message);
    }
}

void GameSession::broadcastActionApplied(const std::string& player_id, const std::string& action, int amount)
{
    const Hand* hand = table_manager_.getCurrentHand();
    if (!hand) return;

    // Find player
    auto player = table_manager_.getPlayer(player_id);
    if (!player) return;

    // Determine next player to act
    std::string next_player_id = "";
    if (hand->current_player_to_act)
    {
        next_player_id = hand->current_player_to_act->id;
    }

    nlohmann::json payload = {
        {"hand_id", hand->id},
        {"player_id", player_id},
        {"action", action},
        {"amount", amount},
        {"new_stack", player->stack},
        {"pot", hand->pot},
        {"next_player_to_act", next_player_id}
    };

    nlohmann::json message = {
        {"type", "action_applied"},
        {"payload", payload}
    };

    broadcastJson(message);
}

void GameSession::broadcastHandCompleted()
{
    const Hand* hand = table_manager_.getCurrentHand();
    if (!hand) return;

    nlohmann::json winners = nlohmann::json::array();
    nlohmann::json pot_distribution = nlohmann::json::array();
    nlohmann::json updated_stacks = nlohmann::json::object();

    // Compute total pot (main + side pots)
    int total_pot = hand->pot;
    for (const SidePot& side_pot : hand->side_pots) {
        total_pot += side_pot.amount;
    }

    // Determine winners (use hand->winners if populated, else compute)
    std::vector<Player*> win_players = hand->winners;
    if (win_players.empty()) {
        win_players = poker::determineWinners(*hand);
    }

    // Prepare winners array and pot distribution
    if (!win_players.empty() && total_pot > 0) {
        int remainder = total_pot % win_players.size();
        int share = (total_pot - remainder) / win_players.size();
        for (size_t i = 0; i < win_players.size(); ++i) {
            Player* winner = win_players[i];
            int amount = share + (i < static_cast<size_t>(remainder) ? 1 : 0);
            // Winner entry
            nlohmann::json winner_json = {
                {"player_id", winner->id},
                {"amount_won", amount},
                {"hand_rank", "unknown"}
            };
            winners.push_back(winner_json);
            // Pot distribution entry (single pot for simplicity)
            nlohmann::json dist_json = {
                {"pot_index", 0},
                {"winner_id", winner->id},
                {"amount", amount}
            };
            pot_distribution.push_back(dist_json);
        }
    }

    // Updated stacks (current stack of each player)
    for (Player* player : hand->players) {
        if (player) {
            updated_stacks[player->id] = player->stack;
        }
    }

    nlohmann::json payload = {
        {"hand_id", hand->id},
        {"winners", winners},
        {"pot_distribution", pot_distribution},
        {"updated_stacks", updated_stacks}
    };

    nlohmann::json message = {
        {"type", "hand_completed"},
        {"payload", payload}
    };

    broadcastJson(message);
}

void GameSession::broadcastPlayerRemoved(const std::string& player_id)
{
    auto player = table_manager_.getPlayer(player_id);
    int seat = (player) ? player->seat : 0;
    nlohmann::json payload = {
        {"player_id", player_id},
        {"seat", seat}
    };
    nlohmann::json message = {
        {"type", "player_removed"},
        {"payload", payload}
    };
    broadcastJson(message);
}

void GameSession::broadcastPlayerReconnected(const std::string& player_id)
{
    nlohmann::json payload = {
        {"player_id", player_id}
    };
    nlohmann::json message = {
        {"type", "player_reconnected"},
        {"payload", payload}
    };
    broadcastJson(message);
}

void GameSession::registerSession(const std::string& player_id, std::shared_ptr<WebSocketSession> session)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    player_sessions_[player_id] = session;
    session_to_player_[session] = player_id;
}

void GameSession::removeSession(const std::string& player_id)
{
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = player_sessions_.find(player_id);
    if (it != player_sessions_.end())
    {
        session_to_player_.erase(it->second);
        player_sessions_.erase(it);
    }
}

void GameSession::onDisconnect(std::shared_ptr<WebSocketSession> session)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "onDisconnect: null session");
        return;
    }

    std::string player_id;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = session_to_player_.find(session);
        if (it == session_to_player_.end())
        {
            return;
        }
        player_id = it->second;
    }

    removeSession(player_id);

    auto player = table_manager_.getPlayer(player_id);
    if (!player)
    {
        return;
    }

    common::log::log(common::log::Level::INFO, "Player disconnected: " + player_id);
    player_state_manager_.onDisconnect(*player);

    nlohmann::json payload = {
        {"player_id", player_id},
        {"remaining_grace_time_ms", disconnect_grace_time_ms_}
    };
    nlohmann::json message = {
        {"type", "player_disconnected"},
        {"payload", payload}
    };
    broadcastJson(message);
}

std::string GameSession::generatePlayerId()
{
    return common::uuid::generate();
}

void GameSession::handleJoin(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "handleJoin: null session");
        return;
    }

    if (!payload.contains("name")) {
        sendJson(session, createErrorResponse("invalid_input", "Missing 'name' field"));
        return;
    }
    std::string name = payload.at("name").get<std::string>();
    if (name.empty()) {
        sendJson(session, createErrorResponse("invalid_input", "Player name cannot be empty"));
        return;
    }

    // Optional player_id for reconnection
    std::string provided_player_id;
    if (payload.contains("player_id"))
    {
        provided_player_id = payload.at("player_id").get<std::string>();
    }

    // If player_id provided, attempt reconnection
    if (!provided_player_id.empty())
    {
        auto player = table_manager_.getPlayer(provided_player_id);
        if (player && (player->connection_status == ConnectionStatus::DISCONNECTED ||
                       player->connection_status == ConnectionStatus::RECONNECTING))
        {
            {
                std::lock_guard<std::mutex> lock(sessions_mutex_);
                auto existing_session_it = player_sessions_.find(provided_player_id);
                if (existing_session_it != player_sessions_.end() && existing_session_it->second != session)
                {
                    sendJson(session, createErrorResponse("player_already_connected", "Player already connected with another session"));
                    return;
                }

                auto current_it = session_to_player_.find(session);
                if (current_it != session_to_player_.end())
                {
                    std::string old_player_id = current_it->second;
                    player_sessions_.erase(old_player_id);
                    session_to_player_.erase(current_it);
                }
                player_sessions_[provided_player_id] = session;
                session_to_player_[session] = provided_player_id;
            }

            // Cancel any disconnection timers
            connection_manager_.cancelTimers(provided_player_id);

            // Update player state
            player_state_manager_.onReconnect(*player);

            common::log::log(common::log::Level::INFO, "Player reconnected: " + provided_player_id);

            // Broadcast reconnection
            broadcastPlayerReconnected(provided_player_id);

            // Send join acknowledgment
            nlohmann::json response = {
                {"type", "join_ack"},
                {"payload", {
                    {"player_id", provided_player_id},
                    {"seat", player->seat}
                }}
            };
            sendJson(session, response);
            return;
        }
        // If player not found or not disconnected, fall through to new player logic
    }

    // Get player_id from session mapping (default welcome-assigned)
    std::string player_id;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = session_to_player_.find(session);
        if (it == session_to_player_.end())
        {
            sendJson(session, createErrorResponse("unauthorized", "Player not registered"));
            return;
        }
        player_id = it->second;
    }

    // Check if player already seated
    auto existing_player = table_manager_.getPlayer(player_id);
    if (existing_player != nullptr)
    {
        // Already seated, send success with current seat
        nlohmann::json response = {
            {"type", "join_ack"},
            {"payload", {
                {"player_id", player_id},
                {"seat", existing_player->seat}
            }}
        };
        sendJson(session, response);
        return;
    }

    // Determine empty seat (0 or 1)
    const Table& table = table_manager_.getTable();
    int seat = -1;
    if (table.seat_1 == nullptr)
    {
        seat = 0;
    }
    else if (table.seat_2 == nullptr)
    {
        seat = 1;
    }
    else
    {
        sendJson(session, createErrorResponse("table_full", "No empty seats available"));
        return;
    }

    // Create player object
    auto player = std::make_shared<Player>();
    player->id = player_id;
    player->name = name;
    player->stack = common::constants::STARTING_STACK;
    player->seat = seat;
    player->hole_cards.clear();
    player->connection_status = ConnectionStatus::CONNECTED;
    player->last_action_timestamp = 0;
    player->disconnected_at = std::nullopt;
    player->is_sitting_out = false;

    common::log::log(common::log::Level::INFO, "New player joined: " + name + " (player_id: " + player_id + ")");

    // Assign seat
    bool success = table_manager_.assignSeat(player, seat);
    if (!success)
    {
        sendJson(session, createErrorResponse("seat_unavailable", "Seat assignment failed"));
        return;
    }

    // Send join acknowledgment
    nlohmann::json response = {
        {"type", "join_ack"},
        {"payload", {
            {"player_id", player_id},
            {"seat", seat}
        }}
    };
    sendJson(session, response);

    // If both seats are now occupied, start a hand
    if (table_manager_.isReadyForHand())
    {
        if (table_manager_.startHand())
        {
            common::log::log(common::log::Level::INFO, "Hand started with both players");
            broadcastHandStarted();
        }
    }
}

void GameSession::handleAction(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "handleAction: null session");
        return;
    }

    if (!payload.contains("hand_id") || !payload.contains("action") || !payload.contains("amount")) {
        sendJson(session, createErrorResponse("invalid_input", "Missing required fields (hand_id, action, amount)"));
        return;
    }
    std::string hand_id = payload.at("hand_id").get<std::string>();
    std::string action = payload.at("action").get<std::string>();
    int amount = payload.at("amount").get<int>();

    if (action != "fold" && action != "call" && action != "raise") {
        sendJson(session, createErrorResponse("invalid_action", "Action must be fold, call, or raise"));
        return;
    }
    if (amount < 0) {
        sendJson(session, createErrorResponse("invalid_amount", "Amount cannot be negative"));
        return;
    }

    // Get player_id from session
    std::string player_id;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = session_to_player_.find(session);
        if (it == session_to_player_.end())
        {
            sendJson(session, createErrorResponse("unauthorized", "Player not registered"));
            return;
        }
        player_id = it->second;
    }

    // Validate hand_id matches current hand
    const Hand* current_hand = table_manager_.getCurrentHand();
    if (!current_hand || current_hand->id != hand_id)
    {
        sendJson(session, createErrorResponse("invalid_hand", "No active hand or hand mismatch"));
        return;
    }

    // Process action via table manager
    bool success = table_manager_.processPlayerAction(player_id, action, amount);
    if (!success)
    {
        common::log::log(common::log::Level::WARN, "Invalid action: " + action + " by player: " + player_id);
        sendJson(session, createErrorResponse("invalid_action", "Action not allowed"));
        return;
    }

    common::log::log(common::log::Level::INFO, "Action processed: " + action + " by player: " + player_id + " amount: " + std::to_string(amount));

    // Action succeeded, broadcast action_applied
    broadcastActionApplied(player_id, action, amount);

    // Get current hand again after processing action
    const Hand* hand_after = table_manager_.getCurrentHand();
    
    // Send action request to next player if hand not completed
    if (hand_after && hand_after->current_player_to_act)
    {
        sendActionRequest(hand_after->current_player_to_act->id);
    }

    // Check if hand is complete
    if (hand_after && poker::isHandComplete(*hand_after)) {
        common::log::log(common::log::Level::INFO, "Hand completed: " + hand_after->id);
        broadcastHandCompleted();
        table_manager_.endHand();
    }
}

void GameSession::handlePing(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "handlePing: null session");
        return;
    }
    nlohmann::json pong = {
        {"type", "pong"},
        {"payload", {}}
    };
    sendJson(session, pong);
}

void GameSession::handleTopUp(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "handleTopUp: null session");
        return;
    }

    // Get player_id from session
    std::string player_id;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = session_to_player_.find(session);
        if (it == session_to_player_.end())
        {
            sendJson(session, createErrorResponse("unauthorized", "Player not registered"));
            return;
        }
        player_id = it->second;
    }

    auto player = table_manager_.getPlayer(player_id);
    if (!player)
    {
        sendJson(session, createErrorResponse("player_not_found", "Player not seated at table"));
        return;
    }

    // Top up if needed
    player->topUp();

    // Send acknowledgment
    nlohmann::json ack = {
        {"type", "top_up_ack"},
        {"payload", {
            {"player_id", player_id},
            {"new_stack", player->stack}
        }}
    };
    sendJson(session, ack);
}

void GameSession::sendJson(std::shared_ptr<WebSocketSession> session, const nlohmann::json& json)
{
    if (!session) {
        common::log::log(common::log::Level::ERROR, "sendJson: null session");
        return;
    }
    session->send(json.dump());
}

void GameSession::broadcastJson(const nlohmann::json& json)
{
    std::string message = json.dump();
    std::vector<std::shared_ptr<WebSocketSession>> sessions_copy;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_copy.reserve(player_sessions_.size());
        for (const auto& pair : player_sessions_)
        {
            sessions_copy.push_back(pair.second);
        }
    }
    for (const auto& session : sessions_copy)
    {
        if (session)
        {
            session->send(message);
        }
    }
}