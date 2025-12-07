#pragma once

#include <string>

class Client {
public:
    Client(const std::string& host, const std::string& port, const std::string& name);
    void run();
    
private:
    std::string host_;
    std::string port_;
    std::string name_;
};