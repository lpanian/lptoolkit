#include "toolkit/format.hh"
#include <gtest/gtest.h>

using namespace lptk;

TEST(FormatContainerTest, Basic)
{
    FormatContainer&& fmt = Format("%0, this is a %1, %2, %3, %4")
        .With("Hello World")
        .With("test")
        .With(1)
        .With(2)
        .With(3u);

    EXPECT_EQ(fmt.GetStr(), "Hello World, this is a test, 1, 2, 3");
}

TEST(FormatContainerTest, PercentChar)
{
    EXPECT_EQ(Format("Test %%%0").With(53).GetStr(), "Test %53");
}

TEST(FormatContainerTest, InvalidIndex)
{
    EXPECT_EQ(Format("Test %03, %0").With(1).GetStr(), "Test %03, 1");
    EXPECT_EQ(Format("Test %abc, %0").With(1).GetStr(), "Test %abc, 1");
}

TEST(FormatContainerTest, ManyArgs)
{
    FormatContainer&& fmt = Format("%0 %1 %2 %3 %4 %5 %6 %7 %8 %9 %10 %11")
        .With(0)
        .With(1)
        .With(2)
        .With(3)
        .With(4)
        .With(5)
        .With(6)
        .With(7)
        .With(8)
        .With(9)
        .With(10)
        .With(11);

    EXPECT_EQ(fmt.GetStr(), "0 1 2 3 4 5 6 7 8 9 10 11");
}

TEST(FormatContainerTest, RepeatedUseArg)
{
    EXPECT_EQ(Format("%0 %1 %0 %2").With("a").With("b").With("c").GetStr(),
        "a b a c");
}


