#include <iostream>
#include "toolkit/timer.hh"

using namespace lptk;

void TestTimer()
{
    Timer timer;

    timer.Start();
    SleepMS(1000);
    timer.Stop();

    std::cout << "Timer times 1000 ms as " << timer.GetTime() << " seconds." << std::endl;
}

void TestSysTime()
{
    SysTime startTime = SysTime::RealTime();
    SleepMS(1000);
    SysTime endTime = SysTime::RealTime();

    std::cout << "SysTime times 1000 ms as " << (endTime - startTime) << " seconds." << std::endl;

    startTime = SysTime::Current();
    for(int i = 0; i < 10; ++i)
        SysTime::Advance(0.1f);
    endTime = SysTime::Current();

    std::cout << "SysTime should be 1.0, is " << (endTime - startTime) << " seconds." << std::endl;
}

int main()
{
    TestTimer();

    TestSysTime();

    return 0;
}

