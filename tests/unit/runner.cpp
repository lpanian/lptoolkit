#include <gtest/gtest.h>
#include <toolkit/mem/new_delete_overload.hh>

using namespace lptk;

int main(int argc, char** argv)
{
    mem_ReportText();
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    mem_ReportText();
    return result;
}

