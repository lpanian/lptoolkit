#include <algorithm>
#include "toolkit/common.hh"
#include "toolkit/dynary.hh"
#include <gtest/gtest.h>

using namespace lptk;

TEST(DynAryTest, BasicTypeTests) {
    DynAry<int> vec;
    vec.push_back(5);
    vec.push_back(6);
    EXPECT_EQ(2u, vec.size());
}

TEST(DynAryTest, CopyConstructor) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(array, array + 10);
    EXPECT_TRUE( std::mismatch(temp.begin(), temp.end(), array).first == temp.end());
    EXPECT_EQ(10u, temp.size());

    DynAry<int> src(5);
    EXPECT_EQ(5u, src.capacity());

    src.push_back(1);
    src.push_back(2);
    src.push_back(3);
    src.push_back(4);
    src.push_back(5);
    DynAry<int> temp2(src.begin(), src.end());
    EXPECT_TRUE( std::mismatch(temp2.begin(), temp2.end(), src.begin()).first == temp2.end());

    DynAry<int> temp3( temp );
    EXPECT_TRUE( std::mismatch(temp.begin(), temp.end(), temp3.begin()).first == temp.end());
    EXPECT_EQ(10u, temp3.size());

}

TEST(DynAryTest, Assignment) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[9] + 1);

    DynAry<int> target ;

    EXPECT_TRUE( target.empty() );
    EXPECT_EQ(0u, target.size());
    EXPECT_EQ(10u, temp.size());

    target = temp;

    EXPECT_TRUE( std::mismatch(target.begin(), target.end(), temp.begin()).first == target.end() );
    EXPECT_EQ(10u, target.size());
}

TEST(DynAryTest, Indexing) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[9] + 1);
    const DynAry<int> ctemp = temp;

    EXPECT_EQ( 42, temp.at(2) );
    EXPECT_EQ( 49, temp.at(9) );

    EXPECT_EQ( 42, ctemp.at(2) );
    EXPECT_EQ( 49, ctemp.at(9) );

    EXPECT_EQ( 42, temp[2] );
    EXPECT_EQ( 49, temp[9] );

    EXPECT_EQ( 42, ctemp[2] );
    EXPECT_EQ( 49, ctemp[9] );

}

TEST(DynAryTest, Resize) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[9] + 1);

    temp.resize(5);
    EXPECT_EQ(5u, temp.size());

    EXPECT_TRUE( std::mismatch(array, array+temp.size(), temp.begin()).first == array + temp.size());

    temp.resize(100);
    EXPECT_EQ(100u, temp.size());
}

TEST(DynAryTest, Reserve) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[9] + 1);

    temp.reserve(100);
    EXPECT_EQ(100u, temp.capacity());
}

TEST(DynAryTest, AssignFunc) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp;
    temp.assign(&array[0], &array[10]);

    EXPECT_TRUE( std::mismatch(temp.begin(), temp.end(), array).first == temp.end());
}

TEST(DynAryTest, Insert) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[10]);

    // insert at beginning
    int oldfirst = temp.front();
    temp.insert(temp.begin(), 100);
    EXPECT_EQ( 100, temp[0] );
    EXPECT_EQ( oldfirst, temp[1] );

    // insert in middle
    int oldmiddle = temp[6];
    temp.insert(temp.begin() + 6, 101);
    EXPECT_EQ( 101, temp[6] );
    EXPECT_EQ( oldmiddle, temp[7] );

    // insert at end
    int oldlast = temp.back();
    temp.insert(temp.end(), 102);
    EXPECT_EQ( oldlast, temp[ temp.size() - 2 ] );
    EXPECT_EQ( 102, temp[ temp.size() - 1] );
}

TEST( DynAryTest, InsertN ) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[10]);

    // insert at beginning
    int oldfirst = temp.front();
    temp.insert(temp.begin(), 2U, 100);
    EXPECT_EQ( 100, temp[0] );
    EXPECT_EQ( 100, temp[1] );
    EXPECT_EQ( oldfirst, temp[2] );

    // insert in middle
    int oldmiddle = temp[6];
    temp.insert(temp.begin() + 6, 2U, 101);
    EXPECT_EQ( 101, temp[6] );
    EXPECT_EQ( 101, temp[7] );
    EXPECT_EQ( oldmiddle, temp[8] );

    // insert at end
    int oldlast = temp.back();
    temp.insert(temp.end(), 2U, 102);
    EXPECT_EQ(oldlast, temp[ temp.size() - 3 ]);
    EXPECT_EQ(102, temp[ temp.size() - 2 ]);
    EXPECT_EQ(102, temp[ temp.size() - 1 ]);
}

TEST( DynAryTest, InsertIterator ) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[10]);

    int stuff[2] = {100,101};

    // insert at beginning
    int oldfirst = temp.front();
    temp.insert(temp.begin(), &stuff[0], &stuff[2]);
    EXPECT_EQ( 100, temp[0] );
    EXPECT_EQ( 101, temp[1] );
    EXPECT_EQ( oldfirst, temp[2] );

    // insert in middle
    int oldmiddle = temp[6];
    temp.insert(temp.begin() + 6, &stuff[0], &stuff[2]);
    EXPECT_EQ( 100, temp[6] );
    EXPECT_EQ( 101, temp[7] );
    EXPECT_EQ( oldmiddle, temp[8] );

    // insert at end
    int oldlast = temp.back();
    temp.insert(temp.end(), &stuff[0], &stuff[2]);
    EXPECT_EQ(oldlast, temp[ temp.size() - 3 ] );
    EXPECT_EQ( 100, temp[ temp.size() - 2] );
    EXPECT_EQ( 101, temp[ temp.size() - 1] );
}

TEST( DynAryTest, Erase ) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[10]);

    // erase at beginning
    temp.erase(temp.begin());
    EXPECT_EQ( 9u, temp.size());
    for(auto v : temp)
        EXPECT_NE(40, v);

    // erase at middle
    temp.erase(temp.begin() + 3);
    EXPECT_EQ( 8u, temp.size() );
    for(auto v : temp)
        EXPECT_NE(44, v);

    // erase at end
    auto endVal = temp.back();
    temp.erase(temp.end() - 1);
    EXPECT_EQ(7u, temp.size());
    for(auto v : temp)
        EXPECT_NE(endVal, v);
}

TEST( DynAryTest, EraseRange) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[10]);

    // erase at beginning (first 2)
    temp.erase(temp.begin(), temp.begin() + 2);
    EXPECT_EQ( 8u, temp.size() );
    for(auto v : temp)
    {
        EXPECT_NE(40, v);
        EXPECT_NE(41, v);
    }

    // erase middle
    temp.erase(temp.begin() + 2, temp.begin() + 4);
    EXPECT_EQ(6u, temp.size());
    for(auto v : temp)
    {
        EXPECT_NE(44, v);
        EXPECT_NE(45, v);
    }

    // erase end
    auto endVal1 = temp[temp.size()-1];
    auto endVal2 = temp[temp.size()-2];
    temp.erase(temp.end() - 2, temp.end());
    EXPECT_EQ(4u, temp.size());
    for(auto v : temp)
    {
        EXPECT_NE(endVal1, v);
        EXPECT_NE(endVal2, v);
    }
}

TEST( DynAryTest, PushBack ) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[10]);

    temp.push_back(1234);
    EXPECT_EQ(11u, temp.size());
    EXPECT_EQ(1234, temp.back());
}

TEST( DynAryTest, PopBack ) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> temp(&array[0],&array[10]);

    temp.pop_back();
    EXPECT_EQ(9u, temp.size());
    EXPECT_EQ(48, temp.back());
}

TEST( DynAryTest, LargeInsert ) {
    int array[10] = {40,41,42,43,44,45,46,47,48,49};
    DynAry<int> insertTemp(&array[0],&array[10]);

    DynAry<int> temp(1);
    temp.resize(1000, 9999);
    EXPECT_EQ( 1000u, temp.size()) ;

    size_t originalCap = insertTemp.capacity();
    insertTemp.insert(insertTemp.end(), temp.begin(), temp.end());
    size_t newCap = insertTemp.capacity();

    EXPECT_TRUE( newCap > originalCap );
}

TEST(DynAryTest, InitializerList) 
{
    DynAry<int> ary{1,2,3,4,5,6};
    EXPECT_EQ(6u, ary.size());
    for(int i = 0; i < 6; ++i)
    {
        EXPECT_EQ(ary[i], i+1);
    }
}


// TODO: nontrivial Type

