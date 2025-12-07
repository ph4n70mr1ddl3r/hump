#pragma once

#include "table_manager.hpp"
#include "websocket_session.hpp"
#include "connection_manager.hpp"
#include "player_state.hpp"
#include "../common/json_serialization.hpp"
#include <memory>
#include <unordered_map>
#include <boost/asio.hpp>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class GameSession : public std::enable_shared_from_this<GameSession> {
public:
    GameSession(boost::asio::io_context& ioc, int action_timeout_ms = 30000, int disconnect_grace_time_ms = 30000, int removal_timeout_ms = 60000);
    
    // Handle incoming message from a WebSocket connection
    void handleMessage(const std::string& message, std::shared_ptr<WebSocketSession> session);
    
    // Send welcome message to a newly connected client
    void sendWelcome(std::shared_ptr<WebSocketSession> session);
    
    // Send hand_started message to all connected clients
    void broadcastHandStarted();
    
    // Send action_request to specific player
    void sendActionRequest(const std::string& player_id);
    
    // Send action_applied to all clients
    void broadcastActionApplied(const std::string& player_id, const std::string& action, int amount);
    
    // Send hand_completed to all clients
    void broadcastHandCompleted();
    
    // Register a WebSocket session for a player
    void registerSession(const std::string& player_id, std::shared_ptr<WebSocketSession> session);
    
    // Remove a session (on disconnect)
    void removeSession(const std::string& player_id);
    
    // Handle WebSocket disconnection
    void onDisconnect(std::shared_ptr<WebSocketSession> session);
    
private:
    TableManager table_manager_;
    std::unordered_map<std::string, std::shared_ptr<WebSocketSession>> player_sessions_;
    std::unordered_map<std::shared_ptr<WebSocketSession>, std::string> session_to_player_;
    
    // Timeout configuration (milliseconds)
    int action_timeout_ms_;
    int disconnect_grace_time_ms_;
    int removal_timeout_ms_;
    
    // Disconnection handling
    boost::asio::io_context& ioc_;
    ConnectionManager connection_manager_;
    PlayerStateManager player_state_manager_;
    
    // Generate a unique player ID for new connections
    std::string generatePlayerId();
    
    // Parse and validate incoming JSON message
    nlohmann::json parseMessage(const std::string& message);
    
    // Handle specific message types
    void handleJoin(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session);
    void handleAction(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session);
    void handlePing(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session);
    void handleTopUp(const nlohmann::json& payload, std::shared_ptr<WebSocketSession> session);
    
    // Send JSON message to a session
    void sendJson(std::shared_ptr<WebSocketSession> session, const nlohmann::json& json);
    
    // Broadcast JSON message to all connected sessions
    void broadcastJson(const nlohmann::json& json);
    
    // Broadcast player_removed message
    void broadcastPlayerRemoved(const std::string& player_id);
};