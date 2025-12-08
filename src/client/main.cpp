#include "client.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port> [name]" << std::endl;
        return 1;
    }
    std::string host = argv[1];
    std::string port = argv[2];
    std::string name = (argc >= 4) ? argv[3] : "Bot";

    Client client(host, port, name);
    client.run();
    return 0;
}