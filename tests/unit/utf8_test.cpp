#include "toolkit/utf8str.hh"
#include <gtest/gtest.h>

using namespace lptk;

////////////////////////////////////////////////////////////////////////////////
TEST(UTF8, Decode8)
{
    const auto utf8 = u8"$";
    auto pair = utf8_decode(utf8);

    EXPECT_EQ('$', pair.first);
    EXPECT_EQ(utf8 + 1, pair.second);
}

TEST(UTF8, Decode16)
{
    const auto utf8 = u8"\u00a2";
    auto pair = utf8_decode(utf8);

    EXPECT_EQ(0xa2, pair.first);
    EXPECT_EQ(utf8 + 2, pair.second);
}

TEST(UTF8, Decode24)
{
    const auto utf8 = u8"\u20ac";
    auto pair = utf8_decode(utf8);

    EXPECT_EQ(0x20ac, pair.first);
    EXPECT_EQ(utf8 + 3, pair.second);
}

TEST(UTF8, Decode32)
{
    const auto utf8 = u8"\U00010348";
    auto pair = utf8_decode(utf8);

    EXPECT_EQ(0x10348, pair.first);
    EXPECT_EQ(utf8 + 4, pair.second);
}

TEST(UTF8, BadContinuation)
{
    const char bad[] = { '\xc1', '\xc0', 0 };
    auto pair = utf8_decode(bad);
    EXPECT_EQ(0, pair.first);
    EXPECT_EQ(bad + 2, pair.second);
}

TEST(UTF8, SimpleLen)
{
    const auto utf8 = u8"caf\u00e9";
    EXPECT_EQ(4, utf8_strlen(utf8));
}

TEST(UTF8, ComboLen)
{
    // we don't combine in our decode or strlen.
    const auto utf8 = u8"cafe\u0301";
    EXPECT_EQ(5, utf8_strlen(utf8));
}

TEST(UTF8, DecodeSequence)
{
    const auto utf8 = u8"caf\u00e9";
   
    int i = 0;
    const CharPoint expected[] = { 'c', 'a', 'f', 0x00e9 };

    auto cur = utf8;
    while (cur && *cur)
    {
        ASSERT(i < ARRAY_SIZE(expected));

        auto pair = utf8_decode(cur);
        EXPECT_EQ(expected[i], pair.first);

        ++i;
        cur = pair.second;
    }
}

TEST(UTF8, RangeIter)
{
    const auto utf8 = u8"caf\u00e9";
   
    int i = 0;
    const CharPoint expected[] = { 'c', 'a', 'f', 0x00e9 };

    for (auto&& cp : utf8range(utf8))
    {
        ASSERT(i < ARRAY_SIZE(expected));
        EXPECT_EQ(expected[i], cp);
        ++i;
    }
}
