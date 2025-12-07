#pragma once

#include <random>
#include <chrono>
#include <thread>

namespace delay {

// Sleep for a random duration between min_ms and max_ms (inclusive)
void randomDelay(int min_ms = 500, int max_ms = 3000);

} // namespace delay