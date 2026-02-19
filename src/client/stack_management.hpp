#pragma once

#include "../common/constants.hpp"

namespace stack_management {

inline bool shouldTopUp(int currentStack) {
    return currentStack < common::constants::BIG_BLIND * 5;
}

inline int topUpAmount(int currentStack) {
    if (!shouldTopUp(currentStack)) return 0;
    return common::constants::STARTING_STACK - currentStack;
}

inline int topUp(int currentStack) {
    if (shouldTopUp(currentStack)) {
        return common::constants::STARTING_STACK;
    }
    return currentStack;
}

} // namespace stack_management
