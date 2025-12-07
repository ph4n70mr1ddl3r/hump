#include "websocket_session.hpp"
#include "game_session.hpp"
#include <iostream>

WebSocketSession::WebSocketSession(tcp::socket socket)
    : ws_(std::move(socket)), is_writing_(false),
      ping_timer_(ws_.get_executor()),
      pong_timeout_timer_(ws_.get_executor()),
      pong_pending_(false)
{
}

void WebSocketSession::setGameSession(std::shared_ptr<GameSession> game_session)
{
    game_session_ = game_session;
}

void WebSocketSession::start()
{
    ws_.async_accept(
        beast::bind_front_handler(
            &WebSocketSession::on_accept,
            shared_from_this()));
}

void WebSocketSession::send(const std::string& message)
{
    net::post(ws_.get_executor(),
        [self = shared_from_this(), message]()
        {
            self->write_queue_.push(message);
            if (!self->is_writing_)
            {
                self->do_write();
            }
        });
}

void WebSocketSession::on_accept(beast::error_code ec)
{
    if (ec)
    {
        std::cerr << "WebSocket accept error: " << ec.message() << std::endl;
        return;
    }
    // Send welcome message to client
    if (auto game_session = game_session_.lock())
    {
        game_session->sendWelcome(shared_from_this());
    }
    do_read();
}

void WebSocketSession::do_read()
{
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketSession::on_read,
            shared_from_this()));
}

void WebSocketSession::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec)
    {
        if (ec != websocket::error::closed)
        {
            std::cerr << "WebSocket read error: " << ec.message() << std::endl;
        }
        // Notify game session about disconnection
        if (auto game_session = game_session_.lock())
        {
            game_session->onDisconnect(shared_from_this());
        }
        return;
    }
    
    // Forward message to game session
    if (auto game_session = game_session_.lock())
    {
        std::string message = beast::buffers_to_string(buffer_.data());
        game_session->handleMessage(message, shared_from_this());
    }
    
    // Clear buffer and read next message
    buffer_.consume(buffer_.size());
    do_read();
}

void WebSocketSession::do_write()
{
    is_writing_ = true;
    ws_.text(true);
    ws_.async_write(
        net::buffer(write_queue_.front()),
        beast::bind_front_handler(
            &WebSocketSession::on_write,
            shared_from_this()));
}

void WebSocketSession::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
    if (ec)
    {
        std::cerr << "WebSocket write error: " << ec.message() << std::endl;
        return;
    }
    
    // Remove the sent message from queue
    write_queue_.pop();
    
    // If there are more messages, send next one
    if (!write_queue_.empty())
    {
        do_write();
    }
    else
    {
        is_writing_ = false;
    }
}