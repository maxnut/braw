#include <gtest/gtest.h>

extern "C" int macro();

TEST(MacroTest, Macro) {
    EXPECT_EQ(macro(), 2);
}