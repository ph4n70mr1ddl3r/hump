#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <queue>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class GameSession; // forward declaration

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    explicit WebSocketSession(tcp::socket socket);
    void start();
    
    // Send a text message to the client
    void send(const std::string& message);
    
    // Set the game session that will handle incoming messages
    void setGameSession(std::shared_ptr<GameSession> game_session);
    
private:
    void on_accept(beast::error_code ec);
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void do_write();
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::weak_ptr<GameSession> game_session_;
    std::queue<std::string> write_queue_;
    bool is_writing_ = false;
};