#pragma once

#include "websocket_session.hpp"
#include "game_session.hpp"
#include <boost/asio.hpp>
#include <memory>

class Server {
public:
    Server(boost::asio::io_context& ioc, unsigned short port, int action_timeout_ms = 30000, int disconnect_grace_time_ms = 30000, int removal_timeout_ms = 60000);
    
private:
    void start_accept();
    
    boost::asio::ip::tcp::acceptor acceptor_;
    int action_timeout_ms_;
    int disconnect_grace_time_ms_;
    int removal_timeout_ms_;
    std::shared_ptr<GameSession> game_session_;
};