#include "websocket_session.hpp"
#include "game_session.hpp"
#include "../common/logging.hpp"
#include <iostream>

WebSocketSession::WebSocketSession(tcp::socket socket)
    : ws_(std::move(socket)), is_writing_(false),
      ping_timer_(ws_.get_executor()),
      pong_timeout_timer_(ws_.get_executor()),
      pong_pending_(false)
{
}

WebSocketSession::~WebSocketSession()
{
    boost::system::error_code ec;
    ping_timer_.cancel(ec);
    pong_timeout_timer_.cancel(ec);
    if (ec && ec != boost::system::errc::operation_not_permitted && ec != boost::asio::error::operation_aborted) {
        common::log::log(common::log::Level::WARN, "WebSocketSession destructor: timer cancel error: " + ec.message());
    }
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
    if (message.empty()) {
        common::log::log(common::log::Level::WARN, "WebSocketSession::send: empty message");
        return;
    }
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
        common::log::log(common::log::Level::ERROR, "WebSocket accept error: " + ec.message());
        return;
    }
    // Send welcome message to client
    if (auto game_session = game_session_.lock())
    {
        game_session->sendWelcome(shared_from_this());
    }
    // Set control callback to handle pong frames
    ws_.control_callback(
        [self = shared_from_this()](beast::websocket::frame_type kind, beast::string_view payload)
        {
            if (kind == beast::websocket::frame_type::pong)
            {
                boost::system::error_code ec;
                self->pong_timeout_timer_.cancel(ec);
                if (ec && ec != boost::asio::error::operation_aborted) {
                    common::log::log(common::log::Level::WARN, "Pong timeout timer cancel error: " + ec.message());
                }
                self->pong_pending_ = false;
            }
        });

    // Start periodic ping timer
    start_ping_timer();

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
            common::log::log(common::log::Level::ERROR, "WebSocket read error: " + ec.message());
        }
        if (auto game_session = game_session_.lock())
        {
            game_session->onDisconnect(shared_from_this());
        }
        return;
    }

    if (auto game_session = game_session_.lock())
    {
        try {
            std::string message = beast::buffers_to_string(buffer_.data());
            if (message.empty()) {
                common::log::log(common::log::Level::WARN, "WebSocketSession::on_read: empty message");
            } else {
                game_session->handleMessage(message, shared_from_this());
            }
        } catch (const std::exception& e) {
            common::log::log(common::log::Level::ERROR, "WebSocketSession::on_read exception: " + std::string(e.what()));
        }
    }

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
        common::log::log(common::log::Level::ERROR, "WebSocket write error: " + ec.message());
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

void WebSocketSession::start_ping_timer()
{
    ping_timer_.expires_after(std::chrono::milliseconds(common::constants::PING_INTERVAL_MS));
    ping_timer_.async_wait(
        beast::bind_front_handler(
            &WebSocketSession::on_ping_timer,
            shared_from_this()));
}

void WebSocketSession::on_ping_timer(beast::error_code ec)
{
    if (ec)
    {
        // Timer cancelled
        return;
    }

    // Start pong timeout timer
    pong_pending_ = true;
    pong_timeout_timer_.expires_after(std::chrono::milliseconds(common::constants::PONG_TIMEOUT_MS));
    pong_timeout_timer_.async_wait(
        beast::bind_front_handler(
            &WebSocketSession::on_pong_timeout,
            shared_from_this()));

    // Send ping
    ws_.async_ping("",
        beast::bind_front_handler(
            &WebSocketSession::on_pong,
            shared_from_this()));
}

void WebSocketSession::on_pong_timeout(beast::error_code ec)
{
    if (ec)
    {
        // Timer cancelled
        return;
    }

    if (pong_pending_)
    {
        // Pong not received in time, treat as disconnect
        if (auto game_session = game_session_.lock())
        {
            game_session->onDisconnect(shared_from_this());
        }
    }
}

void WebSocketSession::on_pong(beast::error_code ec)
{
    if (ec)
    {
        // Ping failed, treat as disconnect
        if (auto game_session = game_session_.lock())
        {
            game_session->onDisconnect(shared_from_this());
        }
        return;
    }

    // Schedule next ping
    start_ping_timer();
}
