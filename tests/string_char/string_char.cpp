#include "gtest/gtest.h"
#include <gtest/gtest.h>

extern "C" const char* hello();
extern "C" char h();
extern "C" char subscript();

TEST(StringTest, Hello) {
    EXPECT_STREQ(hello(), "hello\n");
}
TEST(StringTest, H) {
    EXPECT_EQ(h(), 'h');
}
TEST(StringTest, Subscript) {
    EXPECT_EQ(subscript(), 'h');
}