#include <gtest/gtest.h>

extern "C" int counter();

TEST(RetainTest, Retain) {
    EXPECT_EQ(counter(), 1);
    EXPECT_EQ(counter(), 2);
    EXPECT_EQ(counter(), 3);
}