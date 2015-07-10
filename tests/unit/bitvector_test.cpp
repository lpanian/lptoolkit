#include "toolkit/bitvector.hh"
#include <gtest/gtest.h>

using namespace lptk;

TEST(BitVectorTest, Ctor)
{
	BitVector v(31);

	EXPECT_EQ(v.size(), 31ul);
	for(size_t i = 0; i < v.size(); ++i)
		EXPECT_EQ(v[i], false);

	BitVector v2(11, true);
	for(size_t i = 0; i < v2.size(); ++i)
		EXPECT_EQ(v2[i], true);
}

TEST(BitVectorTest, MoveCtor)
{
	BitVector v;
	v.push_back(true);
	v.push_back(false);
	v.push_back(true);
	
	EXPECT_EQ(v.size(), 3ul);
	EXPECT_EQ(v[0], true);
	EXPECT_EQ(v[1], false);
	EXPECT_EQ(v[2], true);

	BitVector v2 = std::move(v);
	EXPECT_EQ(v.size(), 0ul);
	EXPECT_EQ(v2.size(), 3ul);
	EXPECT_EQ(v2[0], true);
	EXPECT_EQ(v2[1], false);
	EXPECT_EQ(v2[2], true);

}

TEST(BitVectorTest, CopyCtor)
{
	BitVector v;
	v.push_back(true);
	v.push_back(false);
	v.push_back(true);
	BitVector v2 = v;
	EXPECT_EQ(v.size(), 3ul);
	EXPECT_EQ(v[0], true);
	EXPECT_EQ(v[1], false);
	EXPECT_EQ(v[2], true);
	EXPECT_EQ(v2.size(), 3ul);
	EXPECT_EQ(v2[0], true);
	EXPECT_EQ(v2[1], false);
	EXPECT_EQ(v2[2], true);
}

TEST(BitVectorTest, MoveAssign)
{
	BitVector v;
	v.push_back(true);
	v.push_back(false);
	v.push_back(true);
	
	EXPECT_EQ(v.size(), 3ul);
	EXPECT_EQ(v[0], true);
	EXPECT_EQ(v[1], false);
	EXPECT_EQ(v[2], true);

	BitVector v2;
	v2 = std::move(v);
	EXPECT_EQ(v.size(), 0ul);
	EXPECT_EQ(v2.size(), 3ul);
	EXPECT_EQ(v2[0], true);
	EXPECT_EQ(v2[1], false);
	EXPECT_EQ(v2[2], true);
}

TEST(BitVectorTest, Assign)
{
	BitVector v;
	v.push_back(true);
	v.push_back(false);
	v.push_back(true);

	BitVector v2;

	v2 = v;
	EXPECT_EQ(v2.size(), 3ul);
	EXPECT_EQ(v2[0], true);
	EXPECT_EQ(v2[1], false);
	EXPECT_EQ(v2[2], true);
}

TEST(BitVectorTest, SetGet)
{
	BitVector v(9);
	v.set(7, true);
	v.set(8, true);

	const bool expected[9] = {
		false, false, false, false, 
		false, false, false, true, 
		true
	};

	for(size_t i = 0; i < v.size(); ++i) {
		EXPECT_EQ(v.get(i), expected[i]);
	}
}

TEST(BitVectorTest, Resize)
{
	BitVector v(9);
	v.set(7, true);
	v.set(8, true);
	const bool expected[] = {
		false, false, false, false, 
		false, false, false, true, 
		true, false, false, false
	};

	for(size_t i = 0; i < v.size(); ++i) {
		EXPECT_EQ(v.get(i), expected[i]);
	}

	v.resize(12);
	for(size_t i = 0; i < v.size(); ++i) {
		EXPECT_EQ(v.get(i), expected[i]);
	}

	v.resize(4);
	for(size_t i = 0; i < v.size(); ++i) {
		EXPECT_EQ(v.get(i), expected[i]);
	}
}

TEST(BitVectorTest, AtOperator)
{
	BitVector v(9);
	v.set(8, true);
	v.set(9, true);

	for(size_t i = 0; i < v.size(); ++i)
		EXPECT_EQ(v.get(i), v[i]);
}

TEST(BitVectorTest, Swap)
{
	BitVector v1(3);
	v1.set(1, true);

	BitVector v2(13);
	v2.set(12, true);

	v1.swap(v2);

	EXPECT_EQ(v1.size(), 13ul);
	EXPECT_EQ(v2.size(), 3ul);
}
