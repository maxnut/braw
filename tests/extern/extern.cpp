#include <gtest/gtest.h>

extern "C" int i_extern();
extern "C" float f_extern();

extern "C" int i_ext() { return 1; }
extern "C" float f_ext() { return 1.f; }

TEST(ExternTest, IntExtern) {
    EXPECT_EQ(i_extern(), 1);
}
TEST(ExternTest, FloatExtern) {
    EXPECT_EQ(f_extern(), 1.f);
}