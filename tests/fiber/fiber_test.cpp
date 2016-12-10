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
            auto sleepData = reinterpret_cast<const SleepData*>(fiber->GetServiceData());
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
    lptk::fiber::Init(4);
    lptk::fiber::Counter counter;

    
    
    ////////////////////////////////////////
    lptk::DynAry<std::unique_ptr<lptk::fiber::Fiber>> fibers;
    for (size_t i = 0; i < 1000; ++i)
    {
        const auto testCounter = reinterpret_cast<void*>(i);
        fibers.push_back(make_unique<lptk::fiber::Fiber>(&counter, [](void* p)
        {
            const auto num = int(reinterpret_cast<size_t>(p));
            printf("Fiber %d, part A\n", num);
            lptk::fiber::YieldFiber();
            printf("Fiber %d, part B\n", num);
            lptk::fiber::YieldFiber();
            printf("Fiber %d, part C\n", num);

            lptk::fiber::Counter subcounter;
            lptk::fiber::Fiber tmp{ &subcounter, [](void* p) {
                const auto subNum = int(reinterpret_cast<size_t>(p));
                printf("Fiber %d, %d, part A\n", subNum / 1000, subNum);
                lptk::fiber::YieldFiber();
                printf("Fiber %d, %d, part B\n", subNum / 1000, subNum);
                lptk::fiber::YieldFiber();
                printf("Fiber %d, %d, part C\n", subNum / 1000, subNum);
            }, reinterpret_cast<void*>(reinterpret_cast<size_t>(p)*1000)};

            lptk::fiber::WaitForCounter(&subcounter);
        }, testCounter));
    }

    lptk::fiber::WaitForCounter(&counter);

    printf("Finished all fibers.\n");
    fibers.clear();



    ////////////////////////////////////////
    printf("Testing fiber services.\n");
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
    
    for (size_t i = 0; i < 100; ++i)
    {
        ServiceData* serviceData = new ServiceData;
        serviceData->id = int(i);
        serviceData->sleepService = &sleepService;
        serviceData->random = &random;
        serviceData->dist = &dist;

        fibers.push_back(make_unique<lptk::fiber::Fiber>(&counter, [](void *p)
        {
            ServiceData* serviceData = reinterpret_cast<ServiceData*>(p);
            const auto id = serviceData->id;
            {
                const auto sleepService = serviceData->sleepService;
                printf("Fiber %d, before sleep.\n", id);
                const auto beforeTime = Clock::now();
                sleepService->Sleep(1.f + 3.f * (*serviceData->dist)((*serviceData->random)));
                
                const auto timeSince = std::chrono::duration<float>{ Clock::now() - beforeTime }.count();
                printf("Fiber %d, after sleep (slept %f).\n", id, timeSince);

            }
            delete serviceData;
        }, serviceData));
    }

    lptk::fiber::WaitForCounter(&counter);
    printf("Finished all fiber service fibers.\n");

    sleepService.Stop();
    lptk::fiber::Purge();
    return 0;
}


