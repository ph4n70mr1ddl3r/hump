#include "logging.hpp"
#include <iostream>

namespace common {
namespace log {

void init() {
    // Nothing to initialize for now
}

void log(Level level, const std::string& message) {
    // Simple console output
    std::cout << "[" << static_cast<int>(level) << "] " << message << std::endl;
}

} // namespace log
} // namespace common