#include <gtest/gtest.h>

extern "C" float f_nothing();
extern "C" float f_addition();
extern "C" float f_precedence();

TEST(FloatTest, Nothing) {
    EXPECT_EQ(f_nothing(), 0.f);
}
TEST(FloatTest, Addition) {
    EXPECT_EQ(f_addition(), 2.f);
}
TEST(FloatTest, Precedence) {
    EXPECT_EQ(f_precedence(), 5.f);
}