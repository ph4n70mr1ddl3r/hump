#include "client.hpp"
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

Client::Client(const std::string& host, const std::string& port, const std::string& name)
    : host_(host), port_(port), name_(name), player_id_(""), stack_(0)
{
}

#include "../common/json_serialization.hpp"
#include <nlohmann/json.hpp>
#include "random_strategy.hpp"
#include "delay.hpp"
#include "stack_management.hpp"
#include <iostream>
#include <string>

void Client::run()
{
    try
    {
        net::io_context ioc;
        tcp::resolver resolver(ioc);
        auto endpoints = resolver.resolve(host_, port_);

        websocket::stream<tcp::socket> ws(ioc);
        net::connect(ws.next_layer(), endpoints);

        ws.handshake(host_, "/");

        std::cout << "Connected to server at " << host_ << ":" << port_ << std::endl;

        beast::flat_buffer buffer;

        // Receive welcome message
        ws.read(buffer);
        std::string welcome_msg = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size());

        nlohmann::json welcome_json = nlohmann::json::parse(welcome_msg);
        if (welcome_json.at("type") != "welcome")
        {
            std::cerr << "Expected welcome message, got: " << welcome_msg << std::endl;
            return;
        }

        std::string player_id = welcome_json.at("payload").at("player_id").get<std::string>();
        player_id_ = player_id;
        std::cout << "Assigned player ID: " << player_id << std::endl;

        // Send join message
        nlohmann::json join_msg = {
            {"type", "join"},
            {"payload", {
                {"name", name_}
            }}
        };
        ws.write(net::buffer(join_msg.dump()));

        // Receive join acknowledgment
        ws.read(buffer);
        std::string join_ack_msg = beast::buffers_to_string(buffer.data());
        buffer.consume(buffer.size());

        nlohmann::json join_ack_json = nlohmann::json::parse(join_ack_msg);
        if (join_ack_json.at("type") != "join_ack")
        {
            std::cerr << "Expected join_ack, got: " << join_ack_msg << std::endl;
            return;
        }

        int seat = join_ack_json.at("payload").at("seat").get<int>();
        std::cout << "Joined table at seat " << seat << std::endl;

        // Main message loop
        while (ws.is_open())
        {
            ws.read(buffer);
            std::string msg = beast::buffers_to_string(buffer.data());
            buffer.consume(buffer.size());

            nlohmann::json json = nlohmann::json::parse(msg);
            std::string type = json.at("type").get<std::string>();

            if (type == "hand_started")
            {
                std::cout << "Hand started" << std::endl;
                // Could store hand info
            }
            else if (type == "action_request")
            {
                std::string hand_id = json.at("payload").at("hand_id").get<std::string>();
                nlohmann::json possible_actions = json.at("payload").at("possible_actions");
                int call_amount = json.at("payload").at("call_amount").get<int>();
                int min_raise = json.at("payload").at("min_raise").get<int>();
                int max_raise = json.at("payload").at("max_raise").get<int>();

                // Convert possible actions JSON array to vector<string>
                std::vector<std::string> actions;
                for (const auto& action : possible_actions) {
                    actions.push_back(action.get<std::string>());
                }

                // Use random strategy to choose action
                RandomStrategy strategy;
                auto [action, amount] = strategy.chooseAction(actions, call_amount, min_raise, max_raise);

                // Add human-like delay before responding
                delay::randomDelay();

                nlohmann::json action_msg = {
                    {"type", "action"},
                    {"payload", {
                        {"hand_id", hand_id},
                        {"action", action},
                        {"amount", amount}
                    }}
                };
                ws.write(net::buffer(action_msg.dump()));
                std::cout << "Sent action: " << action << " amount " << amount << std::endl;
            }
            else if (type == "action_applied")
            {
                // Just log
                std::cout << "Action applied: " << msg << std::endl;
            }
            else if (type == "hand_completed")
            {
                std::cout << "Hand completed: " << msg << std::endl;
                // Parse updated stacks
                const auto& payload = json.at("payload");
                const auto& updated_stacks = payload.at("updated_stacks");
                if (updated_stacks.contains(player_id_)) {
                    stack_ = updated_stacks.at(player_id_).get<int>();
                    // Check if stack below threshold, send top_up request
                    if (stack_management::shouldTopUp(stack_)) {
                        nlohmann::json top_up_msg = {
                            {"type", "top_up"},
                            {"payload", {}}
                        };
                        ws.write(net::buffer(top_up_msg.dump()));
                        std::cout << "Sent top-up request (stack=" << stack_ << ")" << std::endl;
                    }
                }
            }
            else if (type == "top_up_ack")
            {
                const auto& payload = json.at("payload");
                stack_ = payload.at("new_stack").get<int>();
                std::cout << "Stack topped up to " << stack_ << std::endl;
            }
            else if (type == "error")
            {
                std::cerr << "Server error: " << msg << std::endl;
                break;
            }
            else
            {
                std::cout << "Unknown message type: " << type << std::endl;
            }
        }

        ws.close(websocket::close_code::normal);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Client error: " << e.what() << std::endl;
    }
}