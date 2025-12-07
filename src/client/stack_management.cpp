#include "stack_management.hpp"

namespace stack_management {

bool shouldTopUp(int currentStack) {
    return currentStack < TOP_UP_THRESHOLD_BB * BIG_BLIND;
}

int topUpAmount(int currentStack) {
    const int target = TARGET_STACK_BB * BIG_BLIND;
    if (currentStack >= target) {
        return 0;
    }
    return target - currentStack;
}

int topUp(int currentStack) {
    const int target = TARGET_STACK_BB * BIG_BLIND;
    if (currentStack < target) {
        return target;
    }
    return currentStack;
}

} // namespace stack_management