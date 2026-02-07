#include "logging.hpp"
#include <iostream>

namespace common {
namespace log {

void init() {
    // Nothing to initialize for now
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
    std::cout << "[" << levelToString(level) << "] " << message << std::endl;
}

} // namespace log
} // namespace common