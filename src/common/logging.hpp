#pragma once

#include <string>
#include <mutex>

namespace common {
namespace log {

enum class Level {
    DEBUG,
    INFO,
    WARN,
    ERROR
};

void init();
void log(Level level, const std::string& message);

} // namespace log
} // namespace common
