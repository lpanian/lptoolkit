#include "toolkit/str.hh"
#include <gtest/gtest.h>

using namespace lptk;
using namespace lptk::literals;

TEST(ArrayStringTest, DefaultConstructor) {

    ArrayString<32> str;
    EXPECT_EQ(0u, str.length());
    EXPECT_EQ(32u, str.capacity());
    EXPECT_STREQ("", str.c_str());
}

TEST(StringImplTest, BasicTests)
{
    Str test = "Lucas";
    Str copy = test;

    EXPECT_STREQ("Lucas", test.c_str());
    EXPECT_STREQ("Lucas", copy.c_str());
    EXPECT_TRUE(test == copy);

    test += " Panian";
    EXPECT_STREQ("Lucas Panian", test.c_str());
    EXPECT_STREQ("Lucas", copy.c_str());
    EXPECT_FALSE(test == copy);

    // test reference handling
    Str outerCopy;
    {
        Str scopedTest = "test";
        outerCopy = scopedTest;
        EXPECT_TRUE(outerCopy == scopedTest);
    }
    EXPECT_STREQ("test", outerCopy.c_str());
}

#ifndef _MSC_VER
TEST(StringImplTest, Literal)
{
    Str s = "This is a test"_s;
    EXPECT_STREQ("This is a test", s.c_str());
}
#endif


