#include <gtest/gtest.h>

extern "C" int basic();
extern "C" int dot_chain();
extern "C" int arrow();
extern "C" int arrow_complex();

TEST(StructTest, Basic) {
    EXPECT_EQ(basic(), 1);
}
TEST(StructTest, DotChain) {
    EXPECT_EQ(dot_chain(), 1);
}
TEST(StructTest, Arrow) {
    EXPECT_EQ(arrow(), 1);
}
TEST(StructTest, ArrowComplex) {
    EXPECT_EQ(arrow_complex(), 1);
}