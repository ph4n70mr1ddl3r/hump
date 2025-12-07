#include "server.hpp"
#include <boost/asio.hpp>
#include <iostream>
#include <cstdlib>
#include <string>

int main(int argc, char** argv) {
    unsigned short port = 8080;
    int action_timeout_ms = 30000;
    int disconnect_grace_time_ms = 30000;
    int removal_timeout_ms = 60000;

    // Simple argument parsing
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            port = static_cast<unsigned short>(std::stoi(argv[++i]));
        } else if (arg == "--action-timeout" && i + 1 < argc) {
            action_timeout_ms = std::stoi(argv[++i]);
        } else if (arg == "--disconnect-grace-time" && i + 1 < argc) {
            disconnect_grace_time_ms = std::stoi(argv[++i]);
        } else if (arg == "--removal-timeout" && i + 1 < argc) {
            removal_timeout_ms = std::stoi(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--port <port>] [--action-timeout <ms>] [--disconnect-grace-time <ms>] [--removal-timeout <ms>]\n";
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\n";
            return 1;
        }
    }

    try {
        boost::asio::io_context ioc;
        Server server(ioc, port, action_timeout_ms, disconnect_grace_time_ms, removal_timeout_ms);
        std::cout << "Poker server listening on port " << port << "\n";
        ioc.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}