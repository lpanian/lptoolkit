#include "toolkit/common.hh"
#include "toolkit/binsearch.hh"
#include <gtest/gtest.h>

using namespace lptk;

TEST(BinSearchTest, BasicTest)
{
    const float vals[] = {
        0.f, 1.f, 3.f, 4.f, 6.f, 6.f, 6.f, 7.f
    };

    const auto lowerIt = lptk::binSearchLowerBound(std::begin(vals), std::end(vals), 6.f);
    const auto lowerIndex = lowerIt - std::begin(vals);
    EXPECT_EQ(4, lowerIndex);
    const auto it = lptk::binSearch(std::begin(vals), std::end(vals), 6.f);
    ASSERT(it != std::end(vals));
    EXPECT_EQ(6.f, *it);
}

TEST(BinSearchTest, ImperfectCompareTest)
{
    const float vals[] = {
        0.f, 1.f, 3.f, 4.f, 6.f, 6.f, 6.f, 7.f
    };

    const auto lowerIt = lptk::binSearchLowerBound(std::begin(vals), std::end(vals), 4.5f);
    const auto index = lowerIt - std::begin(vals);
    EXPECT_EQ(4, index);
    const auto it = lptk::binSearch(std::begin(vals), std::end(vals), 4.5f);
    EXPECT_EQ(it, std::end(vals));
}

