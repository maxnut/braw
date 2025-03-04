#include <gtest/gtest.h>

extern "C" double d_nothing();
extern "C" double d_addition();
extern "C" double d_precedence();

TEST(DoubleTest, Nothing) {
    EXPECT_EQ(d_nothing(), 0.0);
}
TEST(DoubleTest, Addition) {
    EXPECT_EQ(d_addition(), 2.0);
}
TEST(DoubleTest, Precedence) {
    EXPECT_EQ(d_precedence(), 5.0);
}