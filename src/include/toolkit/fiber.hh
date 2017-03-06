#pragma once
#ifndef INCLUDED_LPTK_FIBER_HH
#define INCLUDED_LPTK_FIBER_HH

#include <atomic>
#include "toolkit/thread.hh"
#include "toolkit/spinlockqueue.hh"
#include "toolkit/parallel.hh"
        
//////////////////////////////////////////////////////////////////////////////// 
/*
General task system usage:

Initialize by specifying number of worker threads and total number of fibers
and their related resources (stack) to allocate:

    lptk::fiber::FiberInitStruct fiberInit;
    fiberInit.numWorkerThreads = 4;
    fiberInit.numFibers = 32;

    lptk::fiber::Init(fiberInit);

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
            friend class FiberManager;
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
                // If we're subtracting from 0, we just hit 0, so we need to notify our wait queue.
                m_counter.fetch_sub(1u, std::memory_order_acq_rel);
            }

            size_t GetCount() const {
                return m_counter.load(std::memory_order_acquire);
            }

        private:
            std::atomic<size_t> m_counter;
            //lptk::IntrusiveSpinLockQueue<Fiber, FiberNodeTraits> m_waitQueue;
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

            // These must be explicitly called to start and stop the service. They
            // should be called before any fibers make use of the service (somewhere in
            // your init and shutdown).
            void Start();
            void Stop();
            
        protected:
            // tuple struct containing service request data
            struct ServiceRequest
            {
                struct NodeTraits {
                    static ServiceRequest* GetNext(ServiceRequest* ptr) { return ptr->next; }
                    static void SetNext(ServiceRequest* ptr, ServiceRequest* next) { ptr->next = next; }
                };
                void* GetData() { return requestData; }
                ServiceRequest(Fiber* fiber, void* requestData, Counter* counter) :
                    fiber(fiber), requestData(requestData), counter(counter), next(nullptr)
                {}
            private:
                friend class FiberService;
                Fiber* fiber;
                void* requestData;
                Counter* counter;
                ServiceRequest* next;
            };
            
            // Override this function to process requests. The basic loop should
            // pop a fiber, get the fiber request data, process it, and either
            // complete the request or return it to the queue.
            // Return true to keep the thread alive, and false to sleep until there is
            // at least one fiber in the queue.
            virtual bool Update() = 0;
            
            // This callback can be overridden to change the service data when a
            // request has been cancelled by the fiber system - for example, if
            // Stop is called before all fibers have completed. 
            virtual void CancelRequest(ServiceRequest*) {}

            // Enqueues the current fiber with the given request data. 
            // Used to implement the service function.
            void EnqueueRequest(void* requestData);

            // Pops a service fiber from the service queue. 
            ServiceRequest* PopServiceRequest();

            // Returns a service fiber to the service queue, possibly for doing further
            // service work on it.
            void PushServiceFiber(ServiceRequest* request);

            // Used to 'wake' the fiber - returns the task back to the main queue and
            // allows it to resume. This will return execution to just after the EnqueueRequest
            // function.
            void CompleteRequest(ServiceRequest* request);
        private:
            static void RunThread(FiberService* service);
            void Notify();
            void WaitForUpdate();

            lptk::IntrusiveSpinLockQueue<ServiceRequest> m_queue;
            lptk::Thread m_thread;

            std::atomic<bool> m_finished = false;
            std::atomic<bool> m_notified = false;
            Semaphore m_semNotify;
        };
        

        ////////////////////////////////////////////////////////////////////////////////
        using TaskFunc = void(*)(void* userData);
        // Task is a convenient container for the task function pointer and its associated user data
        struct Task
        {
            struct NodeTraits;

            Task() = default;
            Task(TaskFunc fn, void* userData)
                : m_task(fn)
                , m_userData(userData)
            {}
            void Set(TaskFunc fn, void* userData)
            {
                m_task = fn;
                m_userData = userData;
            }

            TaskFunc GetFunc() const { return m_task; }
            void* GetUserData() const { return m_userData; }

            void Execute();
        private:
            friend class FiberManager;
            friend class Fiber;
            void SetCounter(Counter* counter) {
                ASSERT(m_counter == nullptr);
                m_counter = counter;
            }

            TaskFunc m_task = nullptr;
            void* m_userData = nullptr;
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
            // Stack size in bytes for 'small' stack fibers.
            unsigned stackSize = 32 * (1 << 10);
            // Number of small stack fibers in each worker thread.
            unsigned numFibers = 128;
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
        void RunTasks(Task* tasks, size_t numTasks, Counter* counter, int priority = 0);
        
        // cooperative yield - allow us to switch to another fiber.
        void YieldFiber();

        // Returns true if the current thread can execute fibers, and false otherwise.
        // This wil only true for the main thread that Init() was called on and any worker threads.
        bool IsInFiberThread();

        // return index >= 0 for valid worker thread, -1 otherwise.
        int GetFiberThreadId();

        // yield current fiber and keep doing so anytime we are rescheduled until the counter has reached zero.
        void WaitForCounter(Counter* counter);
    }
}

#endif

