#pragma once

namespace common {
namespace constants {

constexpr int SMALL_BLIND = 2;
constexpr int BIG_BLIND = 4;
constexpr int STARTING_STACK = 400;
constexpr int DEFAULT_MIN_RAISE = 4;
constexpr int ACTION_TIMEOUT_MS = 30000;
constexpr int PING_INTERVAL_MS = 30000;
constexpr int PONG_TIMEOUT_MS = 10000;
constexpr int DEFAULT_DEALER_POSITION = 0;
constexpr int SEAT_1 = 0;
constexpr int SEAT_2 = 1;

constexpr int MAX_STACK = 10000;
constexpr int MAX_BET = 10000;
constexpr int MAX_ACTION_TIMEOUT_MS = 300000;
constexpr int MIN_ACTION_TIMEOUT_MS = 1000;

} // namespace constants
} // namespace common
