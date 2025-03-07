#include <gtest/gtest.h>

extern "C" int i_ptr();
extern "C" float f_ptr();
extern "C" int s_ptr();

TEST(PointerTest, IntPtr) {
    EXPECT_EQ(i_ptr(), 1);
}
TEST(PointerTest, FloatPtr) {
    EXPECT_EQ(f_ptr(), 1.f);
}
TEST(PointerTest, StructPtr) {
    EXPECT_EQ(s_ptr(), 1);
}