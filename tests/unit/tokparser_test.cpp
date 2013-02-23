#include "toolkit/tokparser.hh"
#include <gtest/gtest.h>
#include "toolkit/mathcommon.hh"

TEST(TokParserTest, TokensWithoutSpaces)
{
	const char* testStr = 
	"{pi=3.14}";

	TokParser parser(testStr, strlen(testStr));
	char buf[16];
	EXPECT_TRUE(parser.IsTok("{"));
	EXPECT_TRUE(parser.ExpectTok("{"));
	int len = parser.GetSymbol(buf, sizeof(buf));
	EXPECT_EQ(len, 2);
	EXPECT_EQ(strcmp(buf, "pi"), 0);
	EXPECT_TRUE(parser.ExpectTok("="));
	float parsedNumber = parser.GetFloat();
	EXPECT_TRUE(Equal(parsedNumber, 3.14f));
	EXPECT_TRUE(parser.ExpectTok("}"));
}

TEST(TokParserTest, BoolTest)
{
	const char* testStr =
	"boolValue = true";

	TokParser parser(testStr, strlen(testStr));
	char buf[16];
	parser.GetSymbol(buf, sizeof(buf));
	EXPECT_EQ(0, strcmp(buf, "boolValue"));
	EXPECT_TRUE(parser.ExpectTok("="));
	EXPECT_EQ(true, parser.GetBool());
}

TEST(TokParserTest, NumericTest)
{
	const char* testStr = 
		"3.14159";

	{
		TokParser parser(testStr, strlen(testStr));
		EXPECT_EQ(3, parser.GetInt());
	}
	{
		TokParser parser(testStr, strlen(testStr));
		EXPECT_EQ(3U, parser.GetUInt());
	}
	{
		TokParser parser(testStr, strlen(testStr));
		EXPECT_EQ(3.14159f, parser.GetFloat());
	}
	{
		TokParser parser(testStr, strlen(testStr));
		EXPECT_EQ(3.14159, parser.GetDouble());
	}
}

