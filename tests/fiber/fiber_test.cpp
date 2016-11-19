#include <cstdio>
#include <memory>
#include <thread>
#include <toolkit/fiber.hh>
#include <toolkit/dynary.hh>

int main(int, char**)
{
    lptk::fiber::Init(4);
    lptk::fiber::Counter counter;

    lptk::DynAry<std::unique_ptr<lptk::fiber::Fiber>> fibers;
    for (size_t i = 0; i < 10000; ++i)
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

            lptk::fiber::Counter counter;
            lptk::fiber::Fiber tmp{ &counter, [](void* p) {
                const auto subNum = int(reinterpret_cast<size_t>(p));
                printf("Fiber %d, %d, part A\n", subNum / 1000, subNum);
                lptk::fiber::YieldFiber();
                printf("Fiber %d, %d, part B\n", subNum / 1000, subNum);
                lptk::fiber::YieldFiber();
                printf("Fiber %d, %d, part C\n", subNum / 1000, subNum);
            }, reinterpret_cast<void*>(reinterpret_cast<size_t>(p)*1000)};

            lptk::fiber::WaitForCounter(&counter);
        }, testCounter));
    }

    lptk::fiber::WaitForCounter(&counter);

    printf("Finished all fibers.\n");

    lptk::fiber::Purge();
    return 0;
}


