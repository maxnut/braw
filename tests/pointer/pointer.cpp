#include <gtest/gtest.h>

extern "C" int i_ptr();
extern "C" float f_ptr();
extern "C" int s_ptr();
extern "C" int i_assign();
extern "C" float f_assign();
extern "C" int s_assign();

TEST(PointerTest, IntPtr) {
    EXPECT_EQ(i_ptr(), 1);
}
TEST(PointerTest, FloatPtr) {
    EXPECT_EQ(f_ptr(), 1.f);
}
TEST(PointerTest, StructPtr) {
    EXPECT_EQ(s_ptr(), 1);
}
TEST(PointerTest, IntAssign) {
    EXPECT_EQ(i_assign(), 1);
}
TEST(PointerTest, FloatAssign) {
    EXPECT_EQ(f_assign(), 1.f);
}
TEST(PointerTest, StructAssign) {
    EXPECT_EQ(s_assign(), 1);
}