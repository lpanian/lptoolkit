#include "toolkit/quaternion.hh"
#include <gtest/gtest.h>

using namespace lptk;

TEST(QuaternionTest, Ctor)
{
    const quatf basic(1.f, 2.f, 3.f, 4.f);
    EXPECT_EQ(basic.a, 1.f);
    EXPECT_EQ(basic.b, 2.f);
    EXPECT_EQ(basic.c, 3.f);
    EXPECT_EQ(basic.r, 4.f);

    const quatf basic2(v3f(1.f, 2.f, 3.f), 4.f);
    EXPECT_EQ(basic2.a, 1.f);
    EXPECT_EQ(basic2.b, 2.f);
    EXPECT_EQ(basic2.c, 3.f);
    EXPECT_EQ(basic2.r, 4.f);

    const quatf noRot = quatf::no_rotation;
    EXPECT_EQ(noRot, quatf(0.f, 0.f, 0.f, 1.f));

    const quatf copied = basic;
    EXPECT_EQ(copied.a, 1.f);
    EXPECT_EQ(copied.b, 2.f);
    EXPECT_EQ(copied.c, 3.f);
    EXPECT_EQ(copied.r, 4.f);
}

TEST(QuaternionTest, Assign)
{
    const quatf basic(1.f, 2.f, 3.f, 4.f);
    quatf assigned;
    assigned = basic;

    EXPECT_EQ(assigned.a, 1.f);
    EXPECT_EQ(assigned.b, 2.f);
    EXPECT_EQ(assigned.c, 3.f);
    EXPECT_EQ(assigned.r, 4.f);
}

TEST(QuaternionTest, Set)
{
    quatf q;
    q.Set(1.f, 2.f, 3.f, 4.f);
    EXPECT_EQ(q.a, 1.f);
    EXPECT_EQ(q.b, 2.f);
    EXPECT_EQ(q.c, 3.f);
    EXPECT_EQ(q.r, 4.f);
}

TEST(QuaternionTest, Negate)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf qn = -q;

    EXPECT_EQ(qn.a, -1.f);
    EXPECT_EQ(qn.b, -2.f);
    EXPECT_EQ(qn.c, -3.f);
    EXPECT_EQ(qn.r, -4.f);
}

TEST(QuaternionTest, Sub)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf q2(2.f, 4.f, 6.f, 8.f);
    const quatf diff = q - q2;

    EXPECT_EQ(diff.a, -1.f);
    EXPECT_EQ(diff.b, -2.f);
    EXPECT_EQ(diff.c, -3.f);
    EXPECT_EQ(diff.r, -4.f);
}

TEST(QuaternionTest, SubEq)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf q2(2.f, 4.f, 6.f, 8.f);
    quatf diff = q;
    diff -= q2;

    EXPECT_EQ(diff.a, -1.f);
    EXPECT_EQ(diff.b, -2.f);
    EXPECT_EQ(diff.c, -3.f);
    EXPECT_EQ(diff.r, -4.f);
}

TEST(QuaternionTest, Add)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf q2(2.f, 4.f, 6.f, 8.f);
    const quatf sum = q + q2;

    EXPECT_EQ(sum.a, 3.f);
    EXPECT_EQ(sum.b, 6.f);
    EXPECT_EQ(sum.c, 9.f);
    EXPECT_EQ(sum.r, 12.f);
}

TEST(QuaternionTest, AddEq)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf q2(2.f, 4.f, 6.f, 8.f);
    quatf sum = q;
    sum += q2;

    EXPECT_EQ(sum.a, 3.f);
    EXPECT_EQ(sum.b, 6.f);
    EXPECT_EQ(sum.c, 9.f);
    EXPECT_EQ(sum.r, 12.f);
}

TEST(QuaternionTest, Conjugate)
{
    quatf q(1.f, 2.f, 3.f, 4.f);
    q.Conjugate();

    EXPECT_EQ(q.a, -1.f);
    EXPECT_EQ(q.b, -2.f);
    EXPECT_EQ(q.c, -3.f);
    EXPECT_EQ(q.r, 4.f);

    const quatf q2(2.f, 3.f, 4.f, 5.f);
    const quatf conj = Conjugate(q2);
    EXPECT_EQ(conj.a, -2.f);
    EXPECT_EQ(conj.b, -3.f);
    EXPECT_EQ(conj.c, -4.f);
    EXPECT_EQ(conj.r, 5.f);

    const quatf conjProduct = q2 * conj;
    EXPECT_TRUE(Equal(conjProduct, quatf(0,0,0,MagnitudeSq(q2)), 1e-3f));
}

TEST(QuaternionTest, Magnitude)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const float magSq = MagnitudeSq(q);
    const float mag = Magnitude(q);

    EXPECT_EQ(magSq, 1.f + 2.f*2.f + 3.f*3.f + 4.f*4.f);
    EXPECT_EQ(mag, sqrtf(1.f + 2.f*2.f + 3.f*3.f + 4.f*4.f));
}

TEST(QuaternionTest, MultiplyScalar)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf q1 = q * 5.f;
    EXPECT_EQ(q1, quatf(5.f, 10.f, 15.f, 20.f));
    const quatf q2 = 5.f * q;
    EXPECT_EQ(q2, quatf(5.f, 10.f, 15.f, 20.f));
}

TEST(QuaternionTest, Multiply)
{
    // q1 * q2 = s1 * s2 - v1 dot v2 + s1 v2 + s2 v1 + v1 cross v2

    quatf q1(1.f, 2.f, 3.f, 4.f);
    quatf q2(4.f, 3.f, 2.f, 1.f);

    v3f v1 = v3f(q1.a, q1.b, q1.c);
    v3f v2 = v3f(q2.a, q2.b, q2.c);

    quatf result;
    result.r = q1.r * q2.r - Dot(v1, v2);
    v3f vecResult = q1.r * v2 + q2.r * v1 + Cross(v1, v2);
    result.a = vecResult.x;
    result.b = vecResult.y;
    result.c = vecResult.z;

    EXPECT_EQ(result, q1 * q2);
}

TEST(QuaternionTest, Divide)
{
    const quatf q1(1.f, 2.f, 3.f, 4.f);
    const quatf q2(4.f, 3.f, 2.f, 1.f);

    const quatf divResult = q1 / q2;
    const quatf invQ2 = Inverse(q2);
    const quatf manualDivResult = q1 * invQ2;

    EXPECT_TRUE(Equal(divResult, manualDivResult, 1e-3f));
}

TEST(QuaternionTest, DivideScalar)
{
    const quatf q(5.f, 10.f, 15.f, 20.f);
    const quatf q1 = q / 5.f;
    EXPECT_EQ(q1, quatf(1.f, 2.f, 3.f, 4.f));
}

TEST(QuaternionTest, Inverse)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf inv = Inverse(q);

    // test result of multiply
    const quatf prod = q * inv;

    EXPECT_TRUE(Equal(quatf(0,0,0,1), prod, 1e-3f));
}

TEST(QuaternionTest, Normalize)
{
    const quatf q(1.f, 2.f, 3.f, 4.f);
    const quatf normalized = Normalize(q);

    const float mag = Magnitude(q);
    EXPECT_EQ(normalized, q / mag);
}

TEST(QuaternionTest, MakeRotation)
{
    // This is a little tricky to test, since there is more 
    // than one way to rotate a point into the same position. So, we'll treat
    // the rotation as a black box and just look at points that have been rotated.
    const quatf rot90X = MakeRotation(RadFromDeg(90.f), v3f(1,0,0));
    const v3f p1Before(0,1,0);
    const v3f p1After = Rotate(p1Before, rot90X);
    EXPECT_TRUE(p1After.Equal(v3f(0,0,1)));

    const quatf rot90Y = MakeRotation(RadFromDeg(90.f), v3f(0,1,0));
    const v3f p2Before(1, 0, 0);
    const v3f p2After = Rotate(p2Before, rot90Y);
    EXPECT_TRUE(p2After.Equal(v3f(0,0,-1)));

    const quatf rot90Z = MakeRotation(RadFromDeg(90.f), v3f(0,0,1));
    const v3f p3Before(1, 0, 0);
    const v3f p3After = Rotate(p3Before, rot90Z);
    EXPECT_TRUE(p3After.Equal(v3f(0,1,0)));

}

TEST(QuaternionTest, Slerp)
{
    // Test basic rotations with easy angles
    const v3f testPt1(1,0,0);
    const quatf rot45Z = MakeRotation(RadFromDeg(45.f), v3f(0,0,1));
    const quatf rot135Z = MakeRotation(RadFromDeg(135.f), v3f(0,0,1));

    // check that the angle changes linearly
    const int numSteps = 10;
    const v3f startPt = Rotate(testPt1, rot45Z);
    const v3f endPt = Rotate(testPt1, rot135Z);
    for(int i = 0; i < numSteps; ++i)
    {
        const quatf curRot = Slerp(rot45Z, rot135Z, i / float(numSteps - 1));
        const v3f curPt = Rotate(testPt1, curRot);

        const float angle = Atan2(Dot(curPt, endPt), Dot(curPt, startPt));
        EXPECT_TRUE(Equal(angle, RadFromDeg(i * 10.f), 1e-3f));
    }

    // Test 180 Degree rotation
    const quatf rot0X = MakeRotation(0.f, v3f(1,0,0));
    const quatf rot180X = MakeRotation(RadFromDeg(180.f), v3f(1,0,0));

    const quatf mid2 = Slerp(rot0X, rot180X, 0.5f);
    const quatf expectedRot = MakeRotation(RadFromDeg(90.f), v3f(1,0,0));

    const v3f testPt2(0,1,0);
    const v3f result1 = Rotate(testPt2, mid2);
    const v3f result2 = Rotate(testPt2, expectedRot);

    const float dot1 = Dot(result1, testPt2);
    const float dot2 = Dot(result2, testPt2);
    EXPECT_TRUE(dot1 < 1e-3f);
    EXPECT_TRUE(dot2 < 1e-3f);
}

TEST(QuaternionTest, Dot)
{
    const quatf q1(2.f, 3.f, 4.f, 5.f);
    const quatf q2(1.f/2.f, 1.f/3.f, 1.f/4.f, 1.f/5.f);

    const float dotVal = Abs(Dot(q1, q2) - 4.f);
    EXPECT_TRUE(dotVal < 1e-3f);
}

TEST(QuaternionTest, MatrixConversion)
{
    const quatf q = MakeRotation(RadFromDeg(45.f), v3f(0.f, 0.f, 1.f));
    m44f m = q.ToMatrix();

    const quatf q2 = quatf(m);
    const v3f testPt(0,1,0);
    const v3f result1 = Rotate(testPt, q);
    const v3f result2 = Rotate(testPt, q2);

    EXPECT_TRUE(result1.Equal(result2));
}


