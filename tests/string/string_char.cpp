#include "gtest/gtest.h"
#include <gtest/gtest.h>

extern "C" char h();
extern "C" int len(const char* s);
extern "C" char subscript();
extern "C" void print_hello();

TEST(StringTest, H) {
    EXPECT_EQ(h(), 'h');
}
TEST(StringTest, Len) {
    EXPECT_EQ(len("hello\n"), 6);
}
TEST(StringTest, Subscript) {
    EXPECT_EQ(subscript(), 'h');
}
TEST(StringTest, PrintHello) {
    testing::internal::CaptureStdout();
    print_hello();
    EXPECT_EQ(testing::internal::GetCapturedStdout(), "hello\n");
}