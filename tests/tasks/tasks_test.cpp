#include <algorithm>
#include <iostream>
#include "toolkit/taskmgr.hh"
#include "toolkit/thread.hh"
#include "toolkit/timer.hh"

std::atomic<int32_t> g_x;
static constexpr int numTasks = 4095;
static int finished[numTasks];


static void EmptyJob(lptk::task::Task* task, const void*)
{
    ++*reinterpret_cast<int*>(task->m_data);
    ++g_x;
}

static void RootJob(lptk::task::Task* task, const void*p)
{
    EmptyJob(task, p);
}

int main()
{
    lptk::task::Init(std::max(1, lptk::NumProcessors() - 1));

    for (int attemptNum = 0; attemptNum < 500; ++attemptNum)
    {
        std::cout << "attempt " << attemptNum << "\n";
        {
            g_x = 0;
            memset(finished, 0, sizeof(finished));

            lptk::Timer timer;
            timer.Start();
            auto root = lptk::task::CreateTask(RootJob);
            root->m_data = &finished[numTasks - 1];

            for (int i = 0; i < numTasks - 1; ++i)
            {
                auto child = lptk::task::CreateChildTask(root, EmptyJob);
                child->m_data = &finished[i];
                const auto ok = lptk::task::Run(child);
                ASSERT(ok);
            }
            lptk::task::Run(root);
            lptk::task::Wait(root);
            timer.Stop();

            auto const x = g_x.load();
            ASSERT(x == numTasks);
            for (auto x : finished){
                ASSERT(x == 1);
            }
            std::cout << x << " tasks, jobs: " << timer.GetTime() << "\n";
        }
        {
            g_x = 0;
            memset(finished, 0, sizeof(finished));

            lptk::Timer timer;
            timer.Start();

            for (int i = 0; i < numTasks; ++i)
            {
                auto task = lptk::task::CreateTask(EmptyJob);
                ASSERT(task.valid());
                task->m_data = &finished[i];
                lptk::task::Run(task);
                lptk::task::Wait(task);
            }
            timer.Stop();

            auto const x = g_x.load();
            ASSERT(x == numTasks);
            for (auto x : finished)
            {
                ASSERT(x == 1);
            }
            std::cout << x << " tasks, sequential: " << timer.GetTime() << "\n";
        }
    }
    lptk::task::Shutdown();
    return 0;
}

