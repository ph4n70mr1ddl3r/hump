#include "uuid.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace common {
namespace uuid {

std::string generate() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(0, 0xFFFFFFFF);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << dis(gen);
    ss << '-';
    ss << std::setw(4) << (dis(gen) & 0xFFFF);
    ss << '-';
    ss << std::setw(4) << (dis(gen) & 0xFFFF);
    ss << '-';
    ss << std::setw(4) << (dis(gen) & 0xFFFF);
    ss << '-';
    ss << std::setw(12) << dis(gen);
    return ss.str();
}

} // namespace uuid
} // namespace common