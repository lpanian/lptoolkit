#include "toolkit/matrix.hh"
#include "toolkit/matrixutils.hh"
#include "toolkit/mathcommon.hh"
#include <gtest/gtest.h>

////////////////////////////////////////////////////////////////////////////////
TEST(Mat44Test, MemoryOrder)
{
	// assert that we have the same memory layout as opengl
	const m44f m
		(1.f, 2.f, 3.f, 4.f,
		 5.f, 6.f, 7.f, 8.f,
		 9.f, 10.f, 11.f, 12.f,
		 13.f, 14.f, 15.f, 16.f);
	
	EXPECT_EQ(m.Col(0), v4f(1.f, 2.f, 3.f, 4.f));
	EXPECT_EQ(m.Col(1), v4f(5.f, 6.f, 7.f, 8.f));
	EXPECT_EQ(m.Col(2), v4f(9.f, 10.f, 11.f, 12.f));
	EXPECT_EQ(m.Col(3), v4f(13.f, 14.f, 15.f, 16.f));

	EXPECT_EQ(m.Row(0), v4f(1.f, 5.f, 9.f, 13.f));
	EXPECT_EQ(m.Row(1), v4f(2.f, 6.f, 10.f, 14.f));
	EXPECT_EQ(m.Row(2), v4f(3.f, 7.f, 11.f, 15.f));
	EXPECT_EQ(m.Row(3), v4f(4.f, 8.f, 12.f, 16.f));
}

TEST(Mat44Test, Ctor)
{
	const m44f ident = m44f::identity;
	const m44f expected(1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f);
	EXPECT_TRUE(ident.Equal(expected, 0.f));

	const float mem[16] = {1.f, 2.f, 3.f, 4.f, 
		5.f, 6.f, 7.f, 8.f,
		9.f, 10.f, 11.f, 12.f,
		13.f, 14.f, 15.f, 16.f};
	m44f fromMem = mem;

	const m44f memExpected
		(1.f, 2.f, 3.f, 4.f,
		 5.f, 6.f, 7.f, 8.f,
		 9.f, 10.f, 11.f, 12.f,
		 13.f, 14.f, 15.f, 16.f);
	EXPECT_TRUE(fromMem.Equal(memExpected, 0.f));

	const m44f copied = memExpected;
	EXPECT_TRUE(copied.Equal(memExpected, 0.f));
}

TEST(Mat44Test, Assign)
{
	const m44f m
		(1.f, 2.f, 3.f, 4.f,
		 5.f, 6.f, 7.f, 8.f,
		 9.f, 10.f, 11.f, 12.f,
		 13.f, 14.f, 15.f, 16.f);

	m44f assigned;
	assigned = m;
	EXPECT_TRUE(m.Equal(assigned, 0.f));
}

TEST(Mat44Test, Copy)
{
	const m44f m
		(1.f, 2.f, 3.f, 4.f,
		 5.f, 6.f, 7.f, 8.f,
		 9.f, 10.f, 11.f, 12.f,
		 13.f, 14.f, 15.f, 16.f);

	m44f copied;
	copied.Copy(m);
	EXPECT_TRUE(m.Equal(copied, 0.f));
}

TEST(Mat44Test, Multiply)
{
	const m44f m
		(1.f, 2.f, 3.f, 4.f,
		 5.f, 6.f, 7.f, 8.f,
		 9.f, 10.f, 11.f, 12.f,
		 13.f, 14.f, 15.f, 16.f);

	const m44f sumFirstRow
			(1.f, 1.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f);

	const m44f result1 = m * sumFirstRow;
	EXPECT_EQ(result1[0], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result1[1], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result1[2], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result1[3], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i >= 4) 
			EXPECT_EQ(result1[i], 0.f);

	const m44f sumSecondRow
			(0.f, 0.f, 0.f, 0.f,
			1.f, 1.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f);
	const m44f result2 = m * sumSecondRow;
	EXPECT_EQ(result2[4], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result2[5], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result2[6], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result2[7], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i < 4 || i >= 8) 
			EXPECT_EQ(result2[i], 0.f);
	
	const m44f sumThirdRow
			(0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			1.f, 1.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 0.f);
	const m44f result3 = m * sumThirdRow;
	EXPECT_EQ(result3[8], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result3[9], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result3[10], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result3[11], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i < 8 || i >= 12) 
			EXPECT_EQ(result3[i], 0.f);
	
	const m44f sumFourthRow
			(0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			1.f, 1.f, 1.f, 1.f);
	const m44f result4 = m * sumFourthRow;
	EXPECT_EQ(result4[12], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result4[13], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result4[14], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result4[15], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i < 12) 
			EXPECT_EQ(result4[i], 0.f);

}

TEST(Mat44Test, MultiplyEq)
{
	const m44f m
		(1.f, 2.f, 3.f, 4.f,
		 5.f, 6.f, 7.f, 8.f,
		 9.f, 10.f, 11.f, 12.f,
		 13.f, 14.f, 15.f, 16.f);

	const m44f sumFirstRow
			(1.f, 1.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f);

	m44f result1 = m;
	result1 *= sumFirstRow;
	EXPECT_EQ(result1[0], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result1[1], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result1[2], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result1[3], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i >= 4) 
			EXPECT_EQ(result1[i], 0.f);

	const m44f sumSecondRow
			(0.f, 0.f, 0.f, 0.f,
			1.f, 1.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f);
	m44f result2 = m ;
	result2 *= sumSecondRow;
	EXPECT_EQ(result2[4], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result2[5], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result2[6], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result2[7], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i < 4 || i >= 8) 
			EXPECT_EQ(result2[i], 0.f);
	
	const m44f sumThirdRow
			(0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			1.f, 1.f, 1.f, 1.f,
			0.f, 0.f, 0.f, 0.f);
	m44f result3 = m ;
	result3 *= sumThirdRow;
	EXPECT_EQ(result3[8], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result3[9], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result3[10], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result3[11], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i < 8 || i >= 12) 
			EXPECT_EQ(result3[i], 0.f);
	
	const m44f sumFourthRow
			(0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			0.f, 0.f, 0.f, 0.f,
			1.f, 1.f, 1.f, 1.f);
	m44f result4 = m ;
	result4 *= sumFourthRow;
	EXPECT_EQ(result4[12], 1.f + 5.f + 9.f + 13.f);
	EXPECT_EQ(result4[13], 2.f + 6.f + 10.f + 14.f);
	EXPECT_EQ(result4[14], 3.f + 7.f + 11.f + 15.f);
	EXPECT_EQ(result4[15], 4.f + 8.f + 12.f + 16.f);
	for(int i = 0; i < 16; ++i)
		if(i < 12) 
			EXPECT_EQ(result4[i], 0.f);

}

////////////////////////////////////////////////////////////////////////////////
TEST(Mat44UtilsTest, RotationAround)
{
	// this is kinda hard to test in a general case, so just check that the 
	// transform is correct for each axis
	const float theta = 45.f * kDegToRad;
	const float c = Cos(theta);
	const float s = Sin(theta);

	const m44f rotX = RotateAround(v3f(1.f, 0.f, 0.f), theta);	
	const m44f expectedX(
		1.f, 0.f, 0.f, 0.f,
		0.f, c, s, 0.f,
		0.f, -s, c, 0.f,
		0.f, 0.f, 0.f, 1.f
		);
	EXPECT_TRUE(rotX.Equal(expectedX));
	
	const m44f rotY = RotateAround(v3f(0.f, 1.f, 0.f), theta);	
	const m44f expectedY(
		c, 0.f, -s, 0.f,
		0.f, 1.f, 0.f, 0.f,
		s, 0.f, c, 0.f,
		0.f, 0.f, 0.f, 1.f
		);
	EXPECT_TRUE(rotY.Equal(expectedY));
	
	const m44f rotZ = RotateAround(v3f(0.f, 0.f, 1.f), theta);	
	const m44f expectedZ(
		c, s, 0.f, 0.f,
		-s, c, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
		);
	EXPECT_TRUE(rotZ.Equal(expectedZ));
}

TEST(Mat44UtilsTest, MakeTranslation)
{
	const m44f m = MakeTranslation(1.f, 2.f, 3.f);
	const m44f expected (
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		1.f, 2.f, 3.f, 1.f
		);
	EXPECT_TRUE(m.Equal(expected, 0.f));
	
	const m44f m2 = MakeTranslation(v3f(1.f, 2.f, 3.f));
	EXPECT_TRUE(m2.Equal(expected, 0.f));
}

TEST(Mat44UtilsTest, MakeScale)
{
	const m44f m = MakeScale(2.f, 3.f, 4.f);
	const m44f expected(
		2.f, 0.f, 0.f, 0.f,
		0.f, 3.f, 0.f, 0.f,
		0.f, 0.f, 4.f, 0.f,
		0.f, 0.f, 0.f, 1.f);
	EXPECT_TRUE(m.Equal(expected, 0.f));

	const m44f m2 = MakeScale(v3f(2.f, 3.f, 4.f));
	EXPECT_TRUE(m2.Equal(expected, 0.f));

}

TEST(Mat44UtilsTest, MakeCoordinateScale)
{
	const m44f m = MakeCoordinateScale(0.5f, 1.f);
	const m44f expected(
		0.5f, 0.f, 0.f, 0.f,
		0.f, 0.5f, 0.f, 0.f,
		0.f, 0.f, 0.5f, 0.f,
		1.f, 1.f, 1.f, 1.f);
	EXPECT_TRUE(m.Equal(expected, 0.f));
}

TEST(Mat44UtilsTest, MatFromFrame)
{
	const m44f m = MatFromFrame(
		v3f(-1.f, 0.f, 0.f),
		v3f(0.f, -1.f, 0.f),
		v3f(0.f, 0.f, 1.f),
		v3f(1.f, 2.f, 3.f));
	const m44f expected(
		-1.f, 0.f, 0.f, 0.f,
		0.f, -1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		1.f, 2.f, 3.f, 1.f
		);
	EXPECT_TRUE(m.Equal(expected, 0.f));
}

TEST(Mat44UtilsTest, Transpose)
{
	const m44f m(
		1.f, 2.f, 3.f, 4.f,
		 5.f, 6.f, 7.f, 8.f,
		 9.f, 10.f, 11.f, 12.f,
		 13.f, 14.f, 15.f, 16.f
		 );
	const m44f t = Transpose(m);

	EXPECT_EQ(t.Row(0), v4f(1.f, 2.f, 3.f, 4.f));
	EXPECT_EQ(t.Row(1), v4f(5.f, 6.f, 7.f, 8.f));
	EXPECT_EQ(t.Row(2), v4f(9.f, 10.f, 11.f, 12.f));
	EXPECT_EQ(t.Row(3), v4f(13.f, 14.f, 15.f, 16.f));
}

TEST(Mat44UtilsTest, TransformInverse)
{
	const float theta = 60.f * kDegToRad;
	const m44f xform = MakeTranslation(1.f, 2.f, 3.f) *
		RotateAround(v3f(1.f, 0.f, 0.f), theta) *
		MakeScale(2.f, 3.f, 4.f);

	const m44f expectedInverse = 
		MakeScale(1.f/2.f, 1.f/3.f, 1.f/4.f) *
		RotateAround(v3f(1.f, 0.f, 0.f), -theta) *
		MakeTranslation(-1.f, -2.f, -3.f);

	const m44f inv = TransformInverse(xform);

	EXPECT_TRUE(inv.Equal(expectedInverse));
}

TEST(Mat44UtilsTest, TransformVec)
{
	const float theta = 45.f * kDegToRad;
	const float c = Cos(theta);
	const float s = Sin(theta);

	const v3f vec(0.f, 2.f, 0.f);
	const m44f m = MakeTranslation(2.f, 3.f, 4.f) * RotateAround(v3f(1.f, 0.f, 0.f), theta);
	const v3f result = TransformVec(m, vec);

	const v3f expected = v3f(0.f, 2.f * c, 2.f * s);
	EXPECT_TRUE(result.Equal(expected));
}

TEST(Mat44UtilsTest, TransformPoint)
{
	const float theta = 45.f * kDegToRad;
	const float c = Cos(theta);
	const float s = Sin(theta);

	const v3f point(0.f, 2.f, 0.f);
	const m44f m = MakeTranslation(2.f, 3.f, 4.f) * RotateAround(v3f(1.f, 0.f, 0.f), theta);
	const v3f result = TransformPoint(m, point);

	const v3f expected = v3f(2.f, 3.f + 2.f * c, 4.f + 2.f * s);

	EXPECT_TRUE(result.Equal(expected));
}

TEST(Mat44UtilsTest, TransformFloat4)
{
	const float theta = 45.f * kDegToRad;
	const float c = Cos(theta);
	const float s = Sin(theta);

	const float inVec[4] = {0.f, 2.f, 0.f, 0.f};
	const float inPoint[4] = {0.f, 2.f, 0.f, 1.f};

	const m44f m = MakeTranslation(2.f, 3.f, 4.f) * RotateAround(v3f(1.f, 0.f, 0.f), theta);
	float outVec[4], outPoint[4];

	TransformFloat4(outVec, m, inVec);
	TransformFloat4(outPoint, m, inPoint);

	const float expectedVec[4] = {0.f, 2.f * c, 2.f * s, 0.f};
	const float expectedPoint[4] = {2.f, 3.f + 2.f * c, 4.f + 2.f * s, 1.f};

	for(int i = 0; i < 4; ++i)
		EXPECT_TRUE(Equal(outVec[i], expectedVec[i]));
	
	for(int i = 0; i < 4; ++i)
		EXPECT_TRUE(Equal(outPoint[i], expectedPoint[i]));
}
