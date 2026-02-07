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

    // Simple argument parsing (supports spec names: ample-time, removal-timeout)
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            try {
                port = static_cast<unsigned short>(std::stoi(argv[++i]));
            } catch (const std::exception& e) {
                std::cerr << "Invalid port value: " << argv[i] << "\n";
                return 1;
            }
        } else if (arg == "--ample-time" && i + 1 < argc) {
            try {
                disconnect_grace_time_ms = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Invalid disconnect grace time value: " << argv[i] << "\n";
                return 1;
            }
        } else if (arg == "--disconnect-grace-time" && i + 1 < argc) {
            try {
                disconnect_grace_time_ms = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Invalid disconnect grace time value: " << argv[i] << "\n";
                return 1;
            }
        } else if (arg == "--action-timeout" && i + 1 < argc) {
            try {
                action_timeout_ms = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Invalid action timeout value: " << argv[i] << "\n";
                return 1;
            }
        } else if (arg == "--removal-timeout" && i + 1 < argc) {
            try {
                removal_timeout_ms = std::stoi(argv[++i]);
            } catch (const std::exception& e) {
                std::cerr << "Invalid removal timeout value: " << argv[i] << "\n";
                return 1;
            }
        } else if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--port <port>] [--ample-time <seconds>] [--removal-timeout <seconds>] [--action-timeout <ms>]\n";
            std::cout << "Defaults: port=8080, ample-time=30s, removal-timeout=60s, action-timeout=30000ms\n";
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