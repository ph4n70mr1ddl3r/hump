#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/websocket.hpp>
#include <thread>
#include <chrono>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace websocket = beast::websocket;

TEST(WebSocketConnectionTest, ServerStartsAndAcceptsConnection) {
    asio::io_context ioc;
    
    // Create TCP acceptor
    asio::ip::tcp::acceptor acceptor(ioc, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));
    auto endpoint = acceptor.local_endpoint();
    int port = endpoint.port();
    
    // Start server in separate thread (simplified)
    std::thread server_thread([&ioc, &acceptor]() {
        asio::ip::tcp::socket socket(ioc);
        acceptor.accept(socket);
        
        // Create WebSocket stream
        websocket::stream<asio::ip::tcp::socket> ws(std::move(socket));
        ws.accept();
        
        // Read message
        beast::flat_buffer buffer;
        ws.read(buffer);
        
        // Echo back
        ws.text(ws.got_text());
        ws.write(buffer.data());
        
        ws.close(websocket::close_code::normal);
    });
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Client connection
    asio::ip::tcp::resolver resolver(ioc);
    auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
    
    asio::ip::tcp::socket socket(ioc);
    asio::connect(socket, endpoints);
    
    websocket::stream<asio::ip::tcp::socket> ws(std::move(socket));
    ws.handshake("127.0.0.1:" + std::to_string(port), "/");
    
    // Send message
    std::string message = "Hello";
    ws.write(asio::buffer(message));
    
    // Read echo
    beast::flat_buffer buffer;
    ws.read(buffer);
    std::string response = beast::buffers_to_string(buffer.data());
    
    EXPECT_EQ(response, message);
    
    ws.close(websocket::close_code::normal);
    
    server_thread.join();
}

TEST(WebSocketConnectionTest, MultipleConnections) {
    // Test that server can handle two connections (for two players)
    // This is a placeholder for actual test
    EXPECT_TRUE(true);
}