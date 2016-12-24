#pragma once
#ifndef INCLUDED_LPTK_FIBER_HH
#define INCLUDED_LPTK_FIBER_HH

#include <atomic>
#include <condition_variable>
#include <mutex>
#include "toolkit/thread.hh"
#include "toolkit/spinlockqueue.hh"
        
//////////////////////////////////////////////////////////////////////////////// 
/*
General task system usage:

Initialize by specifying number of worker threads and total number of fibers
and their related resources (stack) to allocate:

    lptk::fiber::FiberInitStruct fiberInit;
    fiberInit.numWorkerThreads = 4;
    fiberInit.numHighPriorityWorkerThreads = 1;
    fiberInit.numFibersPerThread = 32;

    lptk::fiber::Init(4, 128);

define a TaskFunc function, which is a void (*)(void*), eg:

    void compute(void* myData)
    {
        auto computeData = reinterpret_cast<MyComputeData*>(myData);
        // this is executed in a fiber
    }

Then setup your tasks:

    lptk::fiber::Task tasks[100];
    MyComputeData computeData[100];
    for(int i = 0; i < 100; ++i)
        tasks[i].Set(compute, &computeData[i]);

Then kick them and wait for results!

    lptk::fiber::Counter taskCounter;
    lptk::fiber::RunTasks(tasks, 100, &taskCounter);
    lptk::fiber::WaitForCounter(taskCounter);
*/

namespace lptk
{
    namespace fiber
    {
        class Fiber;

        struct FiberNodeTraits {
            static Fiber* GetNext(Fiber* ptr);
            static void SetNext(Fiber* ptr, Fiber* next);
        };

        ////////////////////////////////////////////////////////////////////////////////
        class Counter final
        {
        public:
            Counter() : m_counter(0u) {}

            Counter(const Counter&) = delete;
            Counter& operator=(const Counter&) = delete;
            Counter(Counter&&) = delete;
            Counter& operator=(Counter&&) = delete;

            bool IsZero() const { return m_counter.load(std::memory_order_acquire) == 0; }
            void IncRef(size_t count = 1) {
                m_counter.fetch_add(count, std::memory_order_relaxed);
            }
            void DecRef() {
                m_counter.fetch_sub(1u, std::memory_order_acq_rel);
            }

        private:
            std::atomic<size_t> m_counter;
        };



        ////////////////////////////////////////////////////////////////////////////////
        // Fiber services handle 'async' calls that would normally block. When a fiber
        // calls a service, it is put in a special state indicating that it is waiting
        // for the results of the service, and is ignored by the scheduler until 
        // the service wakes up. Periodically when scheduling, if there are fibers in this
        // state, we wake up relevant services and see if they have results available. 
        //
        // FiberServices do not need to return futures, because the entry point call 
        // yields the current fiber. The fiber will not regain control until some result
        // has been computed by the FiberService.
        // 
        // FiberServices get their own thread, so their update function can freely make 
        // blocking calls.
        class FiberService
        {
            friend class FiberManager;
        public:
            FiberService() = default;
            virtual ~FiberService() = default;

            FiberService(const FiberService&) = delete;
            FiberService& operator=(const FiberService&) = delete;
            FiberService(FiberService&&) = delete;
            FiberService& operator=(FiberService&&) = delete;

            void Start();
            void Stop();
            
        protected:
            virtual bool Update() = 0;
            virtual void CancelRequest(Fiber*) {}

            void EnqueueRequest(void* requestData);
            Fiber* PopServiceFiber();
            void PushServiceFiber(Fiber* fiber);
            void CompleteRequest(Fiber* fiber);
            void* GetFiberServiceData(Fiber* fiber);
        private:
            static void RunThread(FiberService* service);
            void Notify();
            void WaitForUpdate();

            lptk::IntrusiveSpinLockQueue<Fiber, FiberNodeTraits> m_queue;
            lptk::Thread m_thread;

            std::atomic<bool> m_finished = false;
            std::condition_variable m_isWaiting;
            std::mutex m_updateMutex;
            bool m_updateRequested = false;
        };
        
        using TaskFunc = void(*)(void* userData);
        // Task is a convenient container for the task function pointer and its associated user data
        struct Task
        {
            struct NodeTraits;

            Task() = default;
            Task(TaskFunc fn, void* userData, bool largeStack = false)
                : m_task(fn)
                , m_userData(userData)
                , m_largeStack(largeStack)
            {}
            void Set(TaskFunc fn, void* userData, bool largeStack = false)
            {
                m_task = fn;
                m_userData = userData;
                m_largeStack = largeStack;
            }
            bool IsLargeStackTask() const { return m_largeStack; }
            TaskFunc GetFunc() const { return m_task; }
            void* GetUserData() const { return m_userData; }

        private:
            friend class FiberManager;
            friend class Fiber;
            void SetCounter(Counter* counter) {
                ASSERT(m_counter == nullptr);
                m_counter = counter;
            }

            TaskFunc m_task = nullptr;
            void* m_userData = nullptr;
            bool m_largeStack = false;
            Task* m_next = nullptr;
            Counter* m_counter = nullptr;
        };


        ////////////////////////////////////////////////////////////////////////////////
        // Specifies the initialization parameters of the fiber system. Must be
        // supplied to Init()
        struct FiberInitStruct
        {
            // Number of worker threads that handle normal and high priority tasks.
            unsigned numWorkerThreads = 1;
            // Number of worker threads dedicated to high priority tasks.
            unsigned numHighPriorityWorkerThreads = 0;
            // Stack size in bytes for 'small' stack fibers.
            unsigned smallFiberStackSize = 32 * (1 << 10);
            // Stack size in bytes for 'large' stack fibers.
            unsigned largeFiberStackSize = 64 * (1 << 10);
            // Number of small stack fibers in each worker thread.
            unsigned numSmallFibersPerThread = 32;
            // Number of small stack fibers in each high priority worker thread (probably smaller).
            unsigned numSmallFibersPerHighPriorityThread = 4;
            // Number of 'large' stack fibers in each worker thread.
            unsigned numLargeFibersPerThread = 4;
            // Number of 'large' stack fibers in each high priority worker thread (probably smaller).
            unsigned numLargeFibersPerHighPriorityThread = 2;
        };

        ////////////////////////////////////////////////////////////////////////////////
        // Initializes fiber system. 
        // Must specify number of worker threaders that execute fibers, and the total
        // number of available fibers.
        // Each fiber requires stack space, so we avoid recreating them.
        bool Init(const FiberInitStruct& init);

        // releases all resources associated with fibers and joins worker threads.
        bool Purge();

        // Run 'small' stack tasks in the normal queue.
        void RunTasks(Task* tasks, size_t numTasks, Counter* counter);
        
        // Run 'small' stack tasks in the high priority queue, which are preferred over normal tasks.
        void RunHighPriorityTasks(Task* tasks, size_t numTasks, Counter* counter);
        
        // cooperative yield - allow us to switch to another fiber.
        void YieldFiber();

        // yield current fiber and keep doing so anytime we are rescheduled until the counter has reached zero.
        // This must not be called in a task! For long-running tasks that need 'child' tasks, the best way
        // is to kick subtasks which re-use the same counter before the current task ends.
        void WaitForCounter(Counter* counter);
    }
}

#endif

