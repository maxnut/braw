#include <gtest/gtest.h>

extern "C" int simple();
extern "C" int ptr();
extern "C" int in_struct();

TEST(ArrayTest, Simple) {
    EXPECT_EQ(simple(), 1);
}
TEST(ArrayTest, Ptr) {
    EXPECT_EQ(ptr(), 1);
}
TEST(ArrayTest, InStruct) {
    EXPECT_EQ(simple(), 1);
}