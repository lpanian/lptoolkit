#include "toolkit/vec.hh"
#include "toolkit/vecutils.hh"
#include <gtest/gtest.h>

using namespace lptk;

////////////////////////////////////////////////////////////////////////////////
TEST(V2fTest, Ctor)
{
	v2f v(1.f, 2.f);
	EXPECT_EQ(v.x, 1.f);
	EXPECT_EQ(v.y, 2.f);
}

TEST(V2fTest, Equal)
{
	const v2f v1(0.f, 0.f);
	const v2f v2(1e-6f, 0.f);
	EXPECT_TRUE(v1.Equal(v2, 1e-3f));
}

TEST(V2fTest, Set)
{
	v2f v;
	v.Set(1.f, 2.f);
	EXPECT_EQ(v.x, 1.f);
	EXPECT_EQ(v.y, 2.f);
}

TEST(V2fTest, Add)
{
	v2f v1(1.f, 1.f );
	v2f v2(2.f, 3.f );
	v2f result = v1 + v2;
	EXPECT_EQ(result, v2f(3.f, 4.f));
}

TEST(V2fTest, AddEq)
{
	v2f v1(1.f, 1.f );
	v2f v2(2.f, 3.f );
	v1 += v2;
	EXPECT_EQ(v1, v2f(3.f, 4.f));
}

TEST(V2fTest, Sub)
{
	v2f v1(1.f, 1.f );
	v2f v2(2.f, 3.f );
	v2f result = v1 - v2;
	EXPECT_EQ(result, v2f(-1.f, -2.f));
}

TEST(V2fTest, SubEq)
{
	v2f v1(1.f, 1.f );
	v2f v2(2.f, 3.f );
	v1 -= v2;
	EXPECT_EQ(v1, v2f(-1.f, -2.f));
}

TEST(V2fTest, Neg)
{
	v2f v1(1.f, 1.f);
	v1 = -v1;
	EXPECT_EQ(v1, v2f(-1.f, -1.f));
}

TEST(V2fTest, Scale)
{
	v2f v1(1.f);
	v2f v2 = 5.f * v1;
	v2f v3 = v1 * 5.f;

	EXPECT_EQ(v2, v2f(5.f, 5.f));
	EXPECT_EQ(v3, v2f(5.f, 5.f));
}

TEST(V2fTest, ScaleEq)
{
	v2f v1(1.f);
	v1 *= 5.f;
	EXPECT_EQ(v1, v2f(5.f, 5.f));
}

TEST(V2fTest, DivScale)
{
	v2f v1(5.f, 5.f);
	v2f v2 = v1 /5.f;
	EXPECT_EQ(v2, v2f(1.f, 1.f));
}

TEST(V2fTest, DivScaleEq)
{
	v2f v1(5.f, 5.f);
	v1 /= 5.f;
	EXPECT_EQ(v1, v2f(1.f, 1.f));
}

TEST(V2fTest, At)
{
	v2f v(1.f, 2.f);
	EXPECT_EQ(v[0], 1.f);
	EXPECT_EQ(v[1], 2.f);
	v[1] = 3.f;
	EXPECT_EQ(v[1], 3.f);

	const v2f v2(1.f, 2.f);
	EXPECT_EQ(v2[0], 1.f);
	EXPECT_EQ(v2[1], 2.f);
}

TEST(V2fTest, Dot)
{
	v2f v1(1.f, 2.f);
	v2f v2(2.f, 1.f);
	EXPECT_EQ(Dot(v1,v2), 4.f);
}

TEST(V2fTest, LengthSq)
{
	v2f v(2.f, 3.f);
	EXPECT_EQ(LengthSq(v), 13.f);
}

TEST(V2fTest, Length)
{
	v2f v(3.f, 4.f);
	EXPECT_EQ(Length(v), sqrtf(3.f*3.f + 4.f*4.f));
}

TEST(V2fTest, Normalize)
{
	v2f v(3.f, 4.f);
	v2f manualN = v / Length(v);
	v2f norm = Normalize(v);
	v2f memberNorm = v;
	memberNorm.Normalize();

	EXPECT_LT(Length(manualN-norm), 1e-6f);
	EXPECT_LT(Length(manualN-memberNorm), 1e-6f);
}

TEST(V2fTest, NormalizeSafe)
{
	v2f v(0.f, 0.f);
	v2f n = NormalizeSafe(v, v2f(1.f, 0.f));
	v2f memberN = v;
	memberN.NormalizeSafe(v2f(1.f, 0.f));

	EXPECT_EQ(n, v2f(1.f, 0.f));
	EXPECT_EQ(memberN, v2f(1.f, 0.f));
}

TEST(V2fTest, Normal)
{
	v2f v(3.f, 4.f);
	v2f n;
	float len;
	Normal(&n, &len, v);

	EXPECT_EQ(n, Normalize(v));
	EXPECT_EQ(len, Length(v));
}

TEST(V2fTest, Min)
{
	v2f v(1.f, 3.f);
	v2f lo(2.f, 2.f);

	v2f m = Min(v, lo);
	EXPECT_EQ(m, v2f(1.f, 2.f));
}

TEST(V2fTest, Max)
{
	v2f v(1.f, 3.f);
	v2f hi(2.f, 2.f);

	v2f m = Max(v, hi);
	EXPECT_EQ(m, v2f(2.f, 3.f));
}


////////////////////////////////////////////////////////////////////////////////
TEST(V3fTest, Ctor)
{
	v3f v(1.f, 2.f, 3.f);
	EXPECT_EQ(v.x, 1.f);
	EXPECT_EQ(v.y, 2.f);
	EXPECT_EQ(v.z, 3.f);
}

TEST(V3fTest, Equal)
{
	const v3f v1(0.f, 0.f, 0.f);
	const v3f v2(1e-6f, 0.f, 0.f);
	EXPECT_TRUE(v1.Equal(v2, 1e-3f));
}

TEST(V3fTest, Set)
{
	v3f v;
	v.Set(1.f, 2.f, 3.f);
	EXPECT_EQ(v.x, 1.f);
	EXPECT_EQ(v.y, 2.f);
	EXPECT_EQ(v.z, 3.f);
}

TEST(V3fTest, Add)
{
	v3f v1(1.f, 1.f, 1.f );
	v3f v2(2.f, 3.f, 4.f );
	v3f result = v1 + v2;
	EXPECT_EQ(result, v3f(3.f, 4.f, 5.f));
}

TEST(V3fTest, AddEq)
{
	v3f v1(1.f, 1.f, 1.f );
	v3f v2(2.f, 3.f, 4.f );
	v1 += v2;
	EXPECT_EQ(v1, v3f(3.f, 4.f, 5.f));
}

TEST(V3fTest, Sub)
{
	v3f v1(1.f, 1.f, 1.f );
	v3f v2(2.f, 3.f, 4.f );
	v3f result = v1 - v2;
	EXPECT_EQ(result, v3f(-1.f, -2.f, -3.f));
}

TEST(V3fTest, SubEq)
{
	v3f v1(1.f, 1.f, 1.f );
	v3f v2(2.f, 3.f, 4.f );
	v1 -= v2;
	EXPECT_EQ(v1, v3f(-1.f, -2.f, -3.f));
}

TEST(V3fTest, Neg)
{
	v3f v1(1.f, 1.f, 1.f);
	v1 = -v1;
	EXPECT_EQ(v1, v3f(-1.f, -1.f, -1.f));
}

TEST(V3fTest, Scale)
{
	v3f v1(1.f);
	v3f v2 = 5.f * v1;
	v3f v3 = v1 * 5.f;

	EXPECT_EQ(v2, v3f(5.f, 5.f, 5.f));
	EXPECT_EQ(v3, v3f(5.f, 5.f, 5.f));
}

TEST(V3fTest, ScaleEq)
{
	v3f v1(1.f);
	v1 *= 5.f;
	EXPECT_EQ(v1, v3f(5.f, 5.f, 5.f));
}

TEST(V3fTest, DivScale)
{
	v3f v1(5.f, 5.f, 5.f);
	v3f v2 = v1 /5.f;
	EXPECT_EQ(v2, v3f(1.f, 1.f, 1.f));
}

TEST(V3fTest, DivScaleEq)
{
	v3f v1(5.f, 5.f, 5.f);
	v1 /= 5.f;
	EXPECT_EQ(v1, v3f(1.f, 1.f, 1.f));
}

TEST(V3fTest, At)
{
	v3f v(1.f, 2.f, 3.f);
	EXPECT_EQ(v[0], 1.f);
	EXPECT_EQ(v[1], 2.f);
	EXPECT_EQ(v[2], 3.f);
	v[1] = 3.f;
	EXPECT_EQ(v[1], 3.f);

	const v3f v2(1.f, 2.f, 3.f);
	EXPECT_EQ(v2[0], 1.f);
	EXPECT_EQ(v2[1], 2.f);
	EXPECT_EQ(v2[2], 3.f);
}

TEST(V3fTest, Dot)
{
	v3f v1(1.f, 2.f, 3.f);
	v3f v2(3.f, 2.f, 1.f);
	EXPECT_EQ(Dot(v1,v2), 10.f);
}

TEST(V3fTest, LengthSq)
{
	v3f v(2.f, 3.f, 4.f);
	EXPECT_EQ(LengthSq(v), 2.f*2.f + 3.f*3.f + 4.f*4.f);
}

TEST(V3fTest, Length)
{
	v3f v(3.f, 4.f, 5.f);
	EXPECT_EQ(Length(v), sqrtf(3.f*3.f + 4.f*4.f + 5.f*5.f));
}

TEST(V3fTest, Normalize)
{
	v3f v(3.f, 4.f, 5.f);
	v3f manualN = v / Length(v);
	v3f norm = Normalize(v);
	v3f memberNorm = v;
	memberNorm.Normalize();

	EXPECT_LT(Length(manualN-norm), 1e-6f);
	EXPECT_LT(Length(manualN-memberNorm), 1e-6f);
}

TEST(V3fTest, NormalizeSafe)
{
	v3f v(0.f, 0.f, 0.f);
	v3f n = NormalizeSafe(v, v3f(1.f, 0.f, 0.f));
	v3f memberN = v;
	memberN.NormalizeSafe(v3f(1.f, 0.f, 0.f));

	EXPECT_EQ(n, v3f(1.f, 0.f, 0.f));
	EXPECT_EQ(memberN, v3f(1.f, 0.f, 0.f));
}

TEST(V3fTest, Normal)
{
	v3f v(3.f, 4.f, 5.f);
	v3f n;
	float len;
	Normal(&n, &len, v);

	EXPECT_EQ(n, Normalize(v));
	EXPECT_EQ(len, Length(v));
}

TEST(V3fTest, Min)
{
	v3f v(1.f, 3.f, 5.f);
	v3f lo(2.f, 2.f, 2.f);

	v3f m = Min(v, lo);
	EXPECT_EQ(m, v3f(1.f, 2.f, 2.f));
}

TEST(V3fTest, Max)
{
	v3f v(1.f, 3.f, 5.f );
	v3f hi(2.f, 2.f, 2.f);

	v3f m = Max(v, hi);
	EXPECT_EQ(m, v3f(2.f, 3.f, 5.f));
}


////////////////////////////////////////////////////////////////////////////////
TEST(V4fTest, Ctor)
{
	v4f v(1.f, 2.f, 3.f, 4.f);
	EXPECT_EQ(v.x, 1.f);
	EXPECT_EQ(v.y, 2.f);
	EXPECT_EQ(v.z, 3.f);
	EXPECT_EQ(v.w, 4.f);
}

TEST(V4fTest, Equal)
{
	const v4f v1(0.f, 0.f, 0.f, 0.f);
	const v4f v2(1e-6f, 0.f, 0.f, 0.f);
	EXPECT_TRUE(v1.Equal(v2, 1e-3f));
}

TEST(V4fTest, Set)
{
	v4f v;
	v.Set(1.f, 2.f, 3.f, 4.f);
	EXPECT_EQ(v.x, 1.f);
	EXPECT_EQ(v.y, 2.f);
	EXPECT_EQ(v.z, 3.f);
	EXPECT_EQ(v.w, 4.f);
}

TEST(V4fTest, Add)
{
	v4f v1(1.f, 1.f, 1.f, 1.f);
	v4f v2(2.f, 3.f, 4.f, 5.f );
	v4f result = v1 + v2;
	EXPECT_EQ(result, v4f(3.f, 4.f, 5.f, 6.f));
}

TEST(V4fTest, AddEq)
{
	v4f v1(1.f, 1.f, 1.f, 1.f );
	v4f v2(2.f, 3.f, 4.f, 5.f );
	v1 += v2;
	EXPECT_EQ(v1, v4f(3.f, 4.f, 5.f, 6.f));
}

TEST(V4fTest, Sub)
{
	v4f v1(1.f, 1.f, 1.f, 1.f );
	v4f v2(2.f, 3.f, 4.f, 5.f );
	v4f result = v1 - v2;
	EXPECT_EQ(result, v4f(-1.f, -2.f, -3.f, -4.f));
}

TEST(V4fTest, SubEq)
{
	v4f v1(1.f, 1.f, 1.f, 1.f);
	v4f v2(2.f, 3.f, 4.f, 5.f );
	v1 -= v2;
	EXPECT_EQ(v1, v4f(-1.f, -2.f, -3.f, -4.f));
}

TEST(V4fTest, Neg)
{
	v4f v1(1.f, 1.f, 1.f, 1.f);
	v1 = -v1;
	EXPECT_EQ(v1, v4f(-1.f, -1.f, -1.f, -1.f));
}

TEST(V4fTest, Scale)
{
	v4f v1(1.f);
	v4f v2 = 5.f * v1;
	v4f v3 = v1 * 5.f;

	EXPECT_EQ(v2, v4f(5.f, 5.f, 5.f, 5.f));
	EXPECT_EQ(v3, v4f(5.f, 5.f, 5.f, 5.f));
}

TEST(V4fTest, ScaleEq)
{
	v4f v1(1.f);
	v1 *= 5.f;
	EXPECT_EQ(v1, v4f(5.f, 5.f, 5.f, 5.f));
}

TEST(V4fTest, DivScale)
{
	v4f v1(5.f, 5.f, 5.f, 5.f);
	v4f v2 = v1 /5.f;
	EXPECT_EQ(v2, v4f(1.f, 1.f, 1.f, 1.f));
}

TEST(V4fTest, DivScaleEq)
{
	v4f v1(5.f, 5.f, 5.f, 5.f);
	v1 /= 5.f;
	EXPECT_EQ(v1, v4f(1.f, 1.f, 1.f, 1.f));
}

TEST(V4fTest, At)
{
	v4f v(1.f, 2.f, 3.f, 4.f);
	EXPECT_EQ(v[0], 1.f);
	EXPECT_EQ(v[1], 2.f);
	EXPECT_EQ(v[2], 3.f);
	EXPECT_EQ(v[3], 4.f);
	v[1] = 3.f;
	EXPECT_EQ(v[1], 3.f);

	const v4f v2(1.f, 2.f, 3.f, 4.f);
	EXPECT_EQ(v2[0], 1.f);
	EXPECT_EQ(v2[1], 2.f);
	EXPECT_EQ(v2[2], 3.f);
	EXPECT_EQ(v2[3], 4.f);
}

TEST(V4fTest, Dot)
{
	v4f v1(1.f, 2.f, 3.f, 4.f);
	v4f v2(4.f, 3.f, 2.f, 1.f);
	EXPECT_EQ(Dot(v1,v2), 4.f + 6.f + 6.f + 4.f);
}

TEST(V4fTest, LengthSq)
{
	v4f v(2.f, 3.f, 4.f, 5.f);
	EXPECT_EQ(LengthSq(v), 2.f*2.f + 3.f*3.f + 4.f*4.f + 5.f*5.f);
}

TEST(V4fTest, Length)
{
	v4f v(3.f, 4.f, 5.f, 6.f);
	EXPECT_EQ(Length(v), sqrtf(3.f*3.f + 4.f*4.f + 5.f*5.f + 6.f*6.f));
}

TEST(V4fTest, Normalize)
{
	v4f v(3.f, 4.f, 5.f, 6.f);
	v4f manualN = v / Length(v);
	v4f norm = Normalize(v);
	v4f memberNorm = v;
	memberNorm.Normalize();

	EXPECT_LT(Length(manualN-norm), 1e-6f);
	EXPECT_LT(Length(manualN-memberNorm), 1e-6f);
}

TEST(V4fTest, NormalizeSafe)
{
	v4f v(0.f, 0.f, 0.f, 0.f);
	v4f n = NormalizeSafe(v, v4f(1.f, 0.f, 0.f, 0.f));
	v4f memberN = v;
	memberN.NormalizeSafe(v4f(1.f, 0.f, 0.f, 0.f));

	EXPECT_EQ(n, v4f(1.f, 0.f, 0.f, 0.f));
	EXPECT_EQ(memberN, v4f(1.f, 0.f, 0.f, 0.f));
}


TEST(V4fTest, Normal)
{
	v4f v(3.f, 4.f, 5.f, 6.f);
	v4f n;
	float len;
	Normal(&n, &len, v);

	EXPECT_EQ(n, Normalize(v));
	EXPECT_EQ(len, Length(v));
}

TEST(V4fTest, Min)
{
	v4f v(1.f, 3.f, 5.f, 6.f);
	v4f lo(2.f, 2.f, 2.f, 2.f);

	v4f m = Min(v, lo);
	EXPECT_EQ(m, v4f(1.f, 2.f, 2.f, 2.f));
}

TEST(V4fTest, Max)
{
	v4f v(1.f, 3.f, 5.f, 6.f );
	v4f hi(2.f, 2.f, 2.f, 2.f);

	v4f m = Max(v, hi);
	EXPECT_EQ(m, v4f(2.f, 3.f, 5.f, 6.f));
}


////////////////////////////////////////////////////////////////////////////////
TEST(VecUtilsTest, RotateAround)
{
	const float theta = 45.f * kDegToRad;
	const float c = Cos(theta);
	const float s = Sin(theta);
	
	const v3f point(0.f, 2.f, 0.f);

	const v3f result = RotateAround(point, v3f(1.f, 0.f, 0.f), theta);
	const v3f expected = v3f(0.f, 2.f * c, 2.f * s);
	EXPECT_TRUE(result.Equal(expected));
}

TEST(VecUtilsTest, CreateArbitraryBasis)
{
	const v3f in(0.f, 0.f, 1.f);
	auto basis = CreateArbitraryBasis(in);

	EXPECT_LT(Dot(std::get<0>(basis), in), 1e-6f);
	EXPECT_LT(Dot(std::get<1>(basis), in), 1e-6f);

	const v3f up = Cross(in, std::get<0>(basis));
	const v3f side = Cross(std::get<1>(basis), in);

	EXPECT_TRUE(side.Equal(std::get<0>(basis)));
	EXPECT_TRUE(up.Equal(std::get<1>(basis)));
}
