#pragma once

#include <string>

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
