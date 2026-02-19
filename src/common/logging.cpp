#include "logging.hpp"
#include <iostream>

namespace common {
namespace log {

static std::mutex log_mutex;

void init() {
}

std::string levelToString(Level level) {
    switch (level) {
        case Level::DEBUG: return "DEBUG";
        case Level::INFO: return "INFO";
        case Level::WARN: return "WARN";
        case Level::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void log(Level level, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << "[" << levelToString(level) << "] " << message << std::endl;
}

} // namespace log
} // namespace common