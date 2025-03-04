#include <gtest/gtest.h>

extern "C" int i_nothing();
extern "C" int i_addition();
extern "C" int i_precedence();

TEST(IntegerTest, Nothing) {
    EXPECT_EQ(i_nothing(), 0);
}
TEST(IntegerTest, Addition) {
    EXPECT_EQ(i_addition(), 2);
}
TEST(IntegerTest, Precedence) {
    EXPECT_EQ(i_precedence(), 5);
}