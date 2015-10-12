#include <algorithm>
#include <iostream>
#include <vector>
#include "toolkit/taskmgr.hh"
#include "toolkit/thread.hh"
#include "toolkit/timer.hh"

static std::atomic<int> g_x;
static void EmptyJob(lptk::task::Task*, const void*)
{
    ++g_x;
}

int main()
{
    constexpr int numTasks = 4095;
    lptk::task::Init(std::max(1, lptk::NumProcessors() - 1));
    {
        g_x = 0;

        lptk::Timer timer;
        timer.Start();

        for (int i = 0; i < numTasks; ++i)
        {
            auto task = lptk::task::CreateTask(EmptyJob);
            ASSERT(task.valid());
            lptk::task::Run(task);
            lptk::task::Wait(task);
        }
        timer.Stop();
    
        auto const x = g_x.load();
        ASSERT(x == numTasks);
        std::cout << x << " tasks, sequential: " << timer.GetTime() << "\n";
    }
    {
        g_x = 0;

        lptk::Timer timer;
        timer.Start();
        auto root = lptk::task::CreateTask(EmptyJob);
        for (int i = 0; i < numTasks - 1; ++i)
        {
            auto child = lptk::task::CreateChildTask(root, EmptyJob);
            lptk::task::Run(child);
        }
        lptk::task::Run(root);
        lptk::task::Wait(root);
        timer.Stop();
  
        auto const x = g_x.load();
        ASSERT(x == numTasks);
        std::cout << x << " tasks, jobs: " << timer.GetTime() << "\n";
    }
    lptk::task::Shutdown();
    return 0;
}

