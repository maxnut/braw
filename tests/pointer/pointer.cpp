#include <gtest/gtest.h>

extern "C" int i_ptr();
extern "C" float f_ptr();

TEST(PointerTest, Nothing) {
    EXPECT_EQ(i_ptr(), 1);
}
TEST(PointerTest, Addition) {
    EXPECT_EQ(f_ptr(), 1.f);
}