#include <cstdio>
#include <memory>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>
#include <toolkit/fiber.hh>
#include <toolkit/dynary.hh>

using Clock = std::chrono::high_resolution_clock;
using TimePoint = decltype(std::chrono::high_resolution_clock::now());

////////////////////////////////////////////////////////////////////////////////
class SleepService : public lptk::fiber::FiberService
{
    struct SleepData
    {
        float m_duration = 0.f;
        TimePoint m_start;
    };

public:
    SleepService() = default;
    ~SleepService() = default;

    void Sleep(float seconds)
    {
        SleepData sleepData;
        sleepData.m_start = Clock::now();
        sleepData.m_duration = seconds;

        // This blocks this fiber until the request is resolved.
        // sleepData is safe on the stack until the fiber
        // wakes up again!
        EnqueueRequest(&sleepData);
    }
protected:
    bool Update() override
    {
        auto minLeft = std::numeric_limits<float>::infinity();

        while(auto fiber = PopServiceFiber())
        { 
            auto sleepData = reinterpret_cast<const SleepData*>(GetFiberServiceData(fiber));
            const auto now = Clock::now();
            const auto timeSince = std::chrono::duration<float>{ now - sleepData->m_start }.count();
            if (timeSince >= sleepData->m_duration)
            {
                // wake that fiber
                CompleteRequest(fiber);
            }
            else
            {
                minLeft = lptk::Min(minLeft, sleepData->m_duration - timeSince);
                PushServiceFiber(fiber);
            }
        }

        if (minLeft != std::numeric_limits<float>::infinity())
        {
            std::this_thread::sleep_for(std::chrono::duration<float>(minLeft));
            return true; // true means we re-enter
        }
        return false; // this means we sleep the thread until a request has been made
    }
};



////////////////////////////////////////////////////////////////////////////////
int main(int, char**)
{
    lptk::fiber::FiberInitStruct fiberInit;
    fiberInit.numWorkerThreads = 4;
    fiberInit.numSmallFibersPerThread = 64;
    fiberInit.numLargeFibersPerThread = 0;
    lptk::fiber::Init(fiberInit);

    lptk::fiber::Counter counter;
    {
        ////////////////////////////////////////
        constexpr int N = 1000;
        lptk::fiber::Task tasks[N];
        size_t i = 0;
        for (auto& task : tasks)
        {
            const auto testCounter = reinterpret_cast<void*>(i++);
            task.Set([](void* p)
            {
                const auto num = int(reinterpret_cast<size_t>(p));
                printf("Fiber %d, part A\n", num);
                lptk::fiber::YieldFiber();
                printf("Fiber %d, part B\n", num);
                lptk::fiber::YieldFiber();
                printf("Fiber %d, part C\n", num);

                // TODO: the current implementation can't handle subtasks- they end up locking things up because
                // we can't allocate more fibers.
                //lptk::fiber::Counter subcounter;
                //lptk::fiber::Task subtask([](void* p) {
                //    const auto subNum = int(reinterpret_cast<size_t>(p));
                //    printf("Fiber %d, %d, part A\n", subNum / 1000, subNum);
                //    lptk::fiber::YieldFiber();
                //    printf("Fiber %d, %d, part B\n", subNum / 1000, subNum);
                //    lptk::fiber::YieldFiber();
                //    printf("Fiber %d, %d, part C\n", subNum / 1000, subNum);
                //}, reinterpret_cast<void*>(reinterpret_cast<size_t>(p) * 1000));

                //lptk::fiber::RunTasks(&subtask, 1, &subcounter);
                //lptk::fiber::WaitForCounter(&subcounter);
            }, testCounter);
        }
        lptk::fiber::RunTasks(tasks, N, &counter);
        lptk::fiber::WaitForCounter(&counter);

        printf("Finished all fibers.\n");
    }


    ////////////////////////////////////////
    printf("Testing fiber services.\n");
    {
        std::random_device rd;
        std::default_random_engine random(rd());
        std::uniform_real_distribution<float> dist(0.f, 1.f);

        SleepService sleepService;
        sleepService.Start();
        struct ServiceData
        {
            int id;
            SleepService* sleepService;

            std::default_random_engine* random;
            std::uniform_real_distribution<float>* dist;
        };

        constexpr int N = 1000;
        ServiceData serviceDataAry[N];
        lptk::fiber::Task serviceTasks[N];

        for (size_t i = 0; i < N; ++i)
        {
            ServiceData* serviceData = &serviceDataAry[i];
            serviceData->id = int(i);
            serviceData->sleepService = &sleepService;
            serviceData->random = &random;
            serviceData->dist = &dist;

            serviceTasks[i].Set([](void* p) {
                ServiceData* serviceData = reinterpret_cast<ServiceData*>(p);
                const auto id = serviceData->id;
                {
                    const auto sleepService = serviceData->sleepService;
                    printf("Fiber %d, before sleep.\n", id);
                    const auto beforeTime = Clock::now();
                    sleepService->Sleep(0.1f + 0.3f * (*serviceData->dist)((*serviceData->random)));

                    const auto timeSince = std::chrono::duration<float>{ Clock::now() - beforeTime }.count();
                    printf("Fiber %d, after sleep (slept %f).\n", id, timeSince);
                }
            }, serviceData);
        }

        lptk::fiber::RunTasks(serviceTasks, N, &counter);

        lptk::fiber::WaitForCounter(&counter);
        printf("Finished all fiber service fibers.\n");
    
        sleepService.Stop();
    }
    lptk::fiber::Purge();
    return 0;
}


