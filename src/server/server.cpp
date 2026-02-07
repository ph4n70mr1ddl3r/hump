#include "server.hpp"
#include "game_session.hpp"
#include "../common/logging.hpp"
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_context& ioc, unsigned short port,
               int action_timeout_ms, int disconnect_grace_time_ms, int removal_timeout_ms)
    : acceptor_(ioc, tcp::endpoint(tcp::v4(), port)),
      action_timeout_ms_(action_timeout_ms),
      disconnect_grace_time_ms_(disconnect_grace_time_ms),
      removal_timeout_ms_(removal_timeout_ms),
      game_session_(std::make_shared<GameSession>(ioc, action_timeout_ms, disconnect_grace_time_ms, removal_timeout_ms))
{
    start_accept();
}

void Server::start_accept()
{
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                auto session = std::make_shared<WebSocketSession>(std::move(socket));
                session->setGameSession(game_session_);
                session->start();
            }
            else
            {
                common::log::log(common::log::Level::ERROR, "Accept error: " + ec.message());
            }
            start_accept();
        });
}