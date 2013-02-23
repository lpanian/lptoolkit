#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	mem_ReportText();
	::testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	mem_ReportText();
	return result;
}

