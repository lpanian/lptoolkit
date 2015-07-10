#include "toolkit/common.hh"
#include "toolkit/binsearch.hh"
#include <gtest/gtest.h>

using namespace lptk;

TEST(BinSearchTest, BasicTest)
{
    const float vals[] = {
        0.f, 1.f, 3.f, 4.f, 6.f, 6.f, 6.f, 7.f
    };

    const size_t initialIndex = 5;
    const auto lowerIndex = lptk::binSearchLower(&vals[0], &vals[0]+ARRAY_SIZE(vals), 6.f, initialIndex);
    EXPECT_EQ(4, lowerIndex);
    const auto index = lptk::binSearch(&vals[0], &vals[0]+ARRAY_SIZE(vals), 6.f, initialIndex);
    ASSERT(index >= 0);
    EXPECT_EQ(6.f, vals[index]);
}

TEST(BinSearchTest, ImperfectCompareTest)
{
    const float vals[] = {
        0.f, 1.f, 3.f, 4.f, 6.f, 6.f, 6.f, 7.f
    };

    const auto index = lptk::binSearchLower(&vals[0], &vals[0]+ARRAY_SIZE(vals), 4.5f);
    EXPECT_EQ(3, index);
    const auto exactIndex = lptk::binSearch(&vals[0], &vals[0]+ARRAY_SIZE(vals), 4.5f);
    EXPECT_EQ(-1, exactIndex);
}

