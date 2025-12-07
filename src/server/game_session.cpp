#include "game_session.hpp"
#include "../common/json_serialization.hpp"
#include "../common/uuid.hpp"
#include <boost/beast.hpp>
#include <iostream>

namespace beast = boost::beast;

GameSession::GameSession(int action_timeout_ms, int disconnect_grace_time_ms, int removal_timeout_ms)
    : action_timeout_ms_(action_timeout_ms),
      disconnect_grace_time_ms_(disconnect_grace_time_ms),
      removal_timeout_ms_(removal_timeout_ms)
{
}

void GameSession::handleMessage(const std::string& message, std::shared_ptr<WebSocketSession> session)
{
    try
    {
        nlohmann::json json = nlohmann::json::parse(message);
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
        else
        {
            // Unknown type, send error
            nlohmann::json error = {
                {"type", "error"},
                {"payload", {
                    {"code", "invalid_message_type"},
                    {"message", "Unknown message type"}
                }}
            };
            sendJson(session, error);
        }
    }
    catch (const nlohmann::json::exception& e)
    {
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "invalid_json"},
                {"message", "Failed to parse JSON"}
            }}
        };
        sendJson(session, error);
    }
    catch (const std::exception& e)
    {
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "internal_error"},
                {"message", "Internal server error"}
            }}
        };
        sendJson(session, error);
    }
}

void GameSession::sendWelcome(std::shared_ptr<WebSocketSession> session)
{
    std::string player_id = generatePlayerId();
    registerSession(player_id, session);
    
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
    int small_blind = 2;
    int big_blind = 4;
    
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
    // For now, assume call amount is big blind (4)
    int call_amount = 4;
    
    // Get min raise from hand
    int min_raise = hand->min_raise;
    
    // Max raise is player's stack
    int max_raise = player->stack;
    
    // Timeout (configurable, default 30 seconds)
    int timeout_ms = 30000;
    
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
    auto session_it = player_sessions_.find(player_id);
    if (session_it != player_sessions_.end())
    {
        sendJson(session_it->second, message);
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
    
    // TODO: compute winners, pot distribution, updated stacks
    nlohmann::json winners = nlohmann::json::array();
    nlohmann::json pot_distribution = nlohmann::json::array();
    nlohmann::json updated_stacks = nlohmann::json::object();
    
    // Placeholder: assume hand completed with no winners
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

void GameSession::registerSession(const std::string& player_id, std::shared_ptr<WebSocketSession> session)
{
    player_sessions_[player_id] = session;
    session_to_player_[session] = player_id;
}

void GameSession::removeSession(const std::string& player_id)
{
    auto it = player_sessions_.find(player_id);
    if (it != player_sessions_.end())
    {
        session_to_player_.erase(it->second);
        player_sessions_.erase(it);
    }
}

std::string GameSession::generatePlayerId()
{
    return common::uuid::generate();
}

nlohmann::json GameSession::parseMessage(const std::string& message)
{
    return nlohmann::json::parse(message);
}

void GameSession::handleJoin(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session)
{
    std::string name = payload.at("name").get<std::string>();
    
    // Get player_id from session mapping
    auto it = session_to_player_.find(session);
    if (it == session_to_player_.end())
    {
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "unauthorized"},
                {"message", "Player not registered"}
            }}
        };
        sendJson(session, error);
        return;
    }
    std::string player_id = it->second;
    
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
        // Both seats occupied
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "table_full"},
                {"message", "No empty seats available"}
            }}
        };
        sendJson(session, error);
        return;
    }
    
    // Create player object
    auto player = std::make_shared<Player>();
    player->id = player_id;
    player->name = name;
    player->stack = 400; // starting stack per spec
    player->seat = seat;
    player->hole_cards.clear();
    player->connection_status = ConnectionStatus::CONNECTED;
    player->last_action_timestamp = 0;
    player->disconnected_at = std::nullopt;
    player->is_sitting_out = false;
    
    // Assign seat
    bool success = table_manager_.assignSeat(player, seat);
    if (!success)
    {
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "seat_unavailable"},
                {"message", "Seat assignment failed"}
            }}
        };
        sendJson(session, error);
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
            broadcastHandStarted();
        }
    }
}

void GameSession::handleAction(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session)
{
    std::string hand_id = payload.at("hand_id").get<std::string>();
    std::string action = payload.at("action").get<std::string>();
    int amount = payload.at("amount").get<int>();
    
    // Get player_id from session
    auto it = session_to_player_.find(session);
    if (it == session_to_player_.end())
    {
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "unauthorized"},
                {"message", "Player not registered"}
            }}
        };
        sendJson(session, error);
        return;
    }
    std::string player_id = it->second;
    
    // Validate hand_id matches current hand
    const Hand* current_hand = table_manager_.getCurrentHand();
    if (!current_hand || current_hand->id != hand_id)
    {
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "invalid_hand"},
                {"message", "No active hand or hand mismatch"}
            }}
        };
        sendJson(session, error);
        return;
    }
    
    // Process action via table manager
    bool success = table_manager_.processPlayerAction(player_id, action, amount);
    if (!success)
    {
        nlohmann::json error = {
            {"type", "error"},
            {"payload", {
                {"code", "invalid_action"},
                {"message", "Action not allowed"}
            }}
        };
        sendJson(session, error);
        return;
    }
    
    // Action succeeded, broadcast action_applied
    broadcastActionApplied(player_id, action, amount);
    
    // Send action request to next player if hand not completed
    if (current_hand->current_player_to_act)
    {
        sendActionRequest(current_hand->current_player_to_act->id);
    }
    // TODO: check if hand completed (all but one folded or showdown)
}

void GameSession::handlePing(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session)
{
    nlohmann::json pong = {
        {"type", "pong"},
        {"payload", {}}
    };
    sendJson(session, pong);
}

void GameSession::sendJson(std::shared_ptr<WebSocketSession> session, const nlohmann::json& json)
{
    session->send(json.dump());
}

void GameSession::broadcastJson(const nlohmann::json& json)
{
    std::string message = json.dump();
    for (const auto& pair : player_sessions_)
    {
        pair.second->send(message);
    }
}