#include "toolkit/hashmap.hh"
#include "toolkit/str.hh"
#include <gtest/gtest.h>

using namespace lptk;

TEST(HashMapTest, BasicTest) 
{
    HashMap<Str, int> map;
    map["Whatup"] = 1234;
    map["World"] = 42;
    EXPECT_TRUE(map.has("World"));
    EXPECT_EQ(1234, map["Whatup"]);
    EXPECT_EQ(42, map["World"]);

    EXPECT_EQ(2u, map.size());
}

TEST(HashMapTest, Regrow)
{
    HashMap<Str, int> map;
    EXPECT_EQ(0u, map.size());
    for(int i = 0; i < 64; ++i)
    {
        Str str;
        Printf(str, "index: %d", i);
        map[str] = i;
    }

    EXPECT_EQ(64u, map.size());

    for(int i = 0; i < 64; ++i)
    {
        Str str;
        Printf(str, "index: %d", i);
        EXPECT_EQ(i, map[str]);
    }

}

TEST(HashMapTest, Delete)
{
    HashMap<Str, int> map;
    for(int i = 0; i < 64; ++i)
    {
        Str str;
        Printf(str, "index: %d", i);
        map[str] = i;
    }

    for(int i = 0; i < 64; ++i)
    {
        Str str;
        Printf(str, "index: %d", i);
        EXPECT_EQ(i, map[str]);
        map.del(str);
    }
}

TEST(HashMapTest, SetAgain)
{
    HashMap<Str, int> map;
    map.set("hi", 2);
    map.set("hi", 3);

    auto value = map.get("hi");
    EXPECT_EQ(value, 3);
    EXPECT_EQ(map.size(), 1u);
}


