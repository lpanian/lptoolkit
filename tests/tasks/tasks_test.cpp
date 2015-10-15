#include <algorithm>
#include <iostream>
#include "toolkit/taskmgr.hh"
#include "toolkit/thread.hh"
#include "toolkit/timer.hh"

std::atomic<int32_t> g_x;
static constexpr int numTasks = 4095;
static int finished[numTasks];


static void EmptyJob(lptk::task::Task* task, const void* data, uint32_t dataSize)
{
    lptk::unused_arg(task);

    auto intPtr = reinterpret_cast<int* const*>(data);
    ASSERT(dataSize >= sizeof(*intPtr));
    ++(**intPtr);
    g_x.fetch_add(1, std::memory_order_acq_rel);
}

static void RootJob(lptk::task::Task* task, const void* data, uint32_t dataSize)
{
    EmptyJob(task, data, dataSize);
}

int main()
{
    lptk::task::Init(std::max(1, lptk::NumProcessors() - 1));

    for (int attemptNum = 0; attemptNum < 500; ++attemptNum)
    {
        std::cout << "attempt " << attemptNum << "\n";
        {
            g_x.store(0, std::memory_order_release);
            memset(finished, 0, sizeof(finished));

            lptk::Timer timer;
            timer.Start();
            auto root = lptk::task::CreateTask(RootJob);

            auto dataPtr = &finished[numTasks - 1];
            root->SetData(&dataPtr, sizeof(dataPtr));

            for (int i = 0; i < numTasks - 1; ++i)
            {
                auto child = lptk::task::CreateChildTask(root, EmptyJob);
                auto dataPtr = &finished[i];
                child->SetData(&dataPtr, sizeof(dataPtr));
                const auto ok = lptk::task::Run(child);
                ASSERT(ok);
            }
            lptk::task::Run(root);
            lptk::task::Wait(root);
            timer.Stop();

            auto const x = g_x.load();
            ASSERT(x == numTasks);
            if (x != numTasks) DEBUGBREAK();
            for (auto x : finished){
                if (x != 1) DEBUGBREAK();
                ASSERT(x == 1);
            }
            std::cout << x << " tasks, jobs: " << timer.GetTime() << "\n";
        }
        {
            g_x.store(0, std::memory_order_release);
            memset(finished, 0, sizeof(finished));

            lptk::Timer timer;
            timer.Start();

            for (int i = 0; i < numTasks; ++i)
            {
                auto task = lptk::task::CreateTask(EmptyJob);
                ASSERT(task.valid());
                auto dataPtr = &finished[i];
                task->SetData(&dataPtr, sizeof(dataPtr));
                lptk::task::Run(task);
                lptk::task::Wait(task);
            }
            timer.Stop();

            auto const x = g_x.load();
            ASSERT(x == numTasks);
            if (x != numTasks) DEBUGBREAK();
            for (auto x : finished)
            {
                if (x != 1) DEBUGBREAK();
                ASSERT(x == 1);
            }
            std::cout << x << " tasks, sequential: " << timer.GetTime() << "\n";
        }
    }
    lptk::task::Shutdown();
    return 0;
}
