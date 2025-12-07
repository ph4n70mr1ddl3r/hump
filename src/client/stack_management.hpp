#pragma once

namespace stack_management {

// Default big blind amount (fixed at 4 chips)
constexpr int BIG_BLIND = 4;

// Default stack top-up threshold (5 big blinds)
constexpr int TOP_UP_THRESHOLD_BB = 5;

// Default target stack (100 big blinds)
constexpr int TARGET_STACK_BB = 100;

// Check if stack is below top-up threshold
bool shouldTopUp(int currentStack);

// Calculate amount needed to top up to target stack
int topUpAmount(int currentStack);

// Top up the stack to target (returns new stack)
int topUp(int currentStack);

} // namespace stack_management