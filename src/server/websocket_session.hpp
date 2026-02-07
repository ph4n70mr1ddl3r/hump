#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <memory>
#include <queue>
#include <atomic>
#include "../common/constants.hpp"

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class GameSession; // forward declaration

class WebSocketSession : public std::enable_shared_from_this<WebSocketSession> {
public:
    explicit WebSocketSession(tcp::socket socket);
    ~WebSocketSession();
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

    // Ping/pong keep-alive
    void start_ping_timer();
    void on_ping_timer(beast::error_code ec);
    void on_pong_timeout(beast::error_code ec);
    void on_pong(beast::error_code ec);

    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    std::weak_ptr<GameSession> game_session_;
    std::queue<std::string> write_queue_;
    std::atomic<bool> is_writing_{false};

    // Ping/pong timers
    net::steady_timer ping_timer_;
    net::steady_timer pong_timeout_timer_;
    std::atomic<bool> pong_pending_{false};
};
