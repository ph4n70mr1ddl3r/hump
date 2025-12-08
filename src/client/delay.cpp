#include "delay.hpp"
#include <random>
#include <chrono>
#include <thread>

namespace delay {

void randomDelay(int min_ms, int max_ms) {
    if (min_ms > max_ms) {
        std::swap(min_ms, max_ms);
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min_ms, max_ms);
    int delay_ms = dist(gen);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
}

} // namespace delay