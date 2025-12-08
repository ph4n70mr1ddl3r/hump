#include <gtest/gtest.h>
#include "stack_management.hpp"

namespace sm = stack_management;

TEST(StackManagementTest, ShouldTopUpBelowThreshold) {
    EXPECT_TRUE(sm::shouldTopUp(0));
    EXPECT_TRUE(sm::shouldTopUp(10));
    EXPECT_TRUE(sm::shouldTopUp(19));
}

TEST(StackManagementTest, ShouldTopUpAtOrAboveThreshold) {
    EXPECT_FALSE(sm::shouldTopUp(20));
    EXPECT_FALSE(sm::shouldTopUp(100));
    EXPECT_FALSE(sm::shouldTopUp(400));
    EXPECT_FALSE(sm::shouldTopUp(1000));
}

TEST(StackManagementTest, TopUpAmount) {
    EXPECT_EQ(sm::topUpAmount(0), 400);
    EXPECT_EQ(sm::topUpAmount(10), 390);
    EXPECT_EQ(sm::topUpAmount(19), 381);
    EXPECT_EQ(sm::topUpAmount(20), 380);
    EXPECT_EQ(sm::topUpAmount(399), 1);
    EXPECT_EQ(sm::topUpAmount(400), 0);
    EXPECT_EQ(sm::topUpAmount(401), 0); // already above target, no top-up needed
}

TEST(StackManagementTest, TopUp) {
    EXPECT_EQ(sm::topUp(0), 400);
    EXPECT_EQ(sm::topUp(10), 400);
    EXPECT_EQ(sm::topUp(19), 400);
    EXPECT_EQ(sm::topUp(20), 400);
    EXPECT_EQ(sm::topUp(399), 400);
    EXPECT_EQ(sm::topUp(400), 400);
    EXPECT_EQ(sm::topUp(401), 401); // no change
}