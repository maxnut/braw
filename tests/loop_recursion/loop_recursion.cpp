#include <gtest/gtest.h>

extern "C" int sum(int n);
extern "C" int factorial(int n);

TEST(LoopRecursionTest, Loop) {
    EXPECT_EQ(sum(3), 3);
}
TEST(LoopRecursionTest, Recursion) {
    EXPECT_EQ(factorial(5), 120);
}