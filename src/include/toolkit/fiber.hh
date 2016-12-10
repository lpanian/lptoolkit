#pragma once
#ifndef INCLUDED_LPTK_FIBER_HH
#define INCLUDED_LPTK_FIBER_HH

#include <atomic>
#include <condition_variable>
#include <mutex>
#include "toolkit/thread.hh"

namespace lptk
{
    namespace fiber
    {
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
            void IncRef() {
                m_counter.fetch_add(1u, std::memory_order_relaxed);
            }
            void DecRef() {
                m_counter.fetch_sub(1u, std::memory_order_acq_rel);
            }

        private:
            std::atomic<unsigned> m_counter;
        };

        
        
        ////////////////////////////////////////////////////////////////////////////////
        using FiberFunc = void(*)(void* userData);
        class Fiber 
        {
            friend class FiberManager;
            friend class FiberService;
        public:
            Fiber(Counter* counter, FiberFunc fn, void* userData);

            Fiber(const Fiber&) = delete;
            Fiber& operator=(const Fiber&) = delete;
            Fiber(Fiber&& other) = delete;
            Fiber& operator=(Fiber&& other) = delete;

            ~Fiber();

            void* GetServiceData() const { return m_serviceData; }
        protected:
            void Run();

            Counter* m_counter = nullptr;
            FiberFunc m_fn = nullptr;
            void* m_userData = nullptr;
            Fiber* m_next = nullptr;
            void* m_serviceData = nullptr;
            int m_serviceThreadIndex = -1;

#if defined(WINDOWS)
            static void CALLBACK FiberMain(void* param);
            // actual implementation pointer (void* on windows)
            void* m_fiber = nullptr;
#elif defined(LINUX)
#error LinuxFiber implementation not yet implemented. You should do that!
#endif
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

            bool EnqueueRequest(void* requestData);
            Fiber* PopServiceFiber();
            void PushServiceFiber(Fiber* fiber);
            void CompleteRequest(Fiber* fiber);
        private:
            static void RunThread(FiberService* service);
            void Notify();
            void WaitForUpdate();

            std::atomic<Fiber*> m_waiting = nullptr;
            std::atomic<Fiber*> m_queue = nullptr;
            std::atomic<bool> m_finished = false;
            lptk::Thread m_thread;

            std::condition_variable m_isWaiting;
            std::mutex m_updateMutex;
            bool m_updateRequested = false;
        };


        ////////////////////////////////////////////////////////////////////////////////
        // Initialize fiber worker threads
        bool Init(int numWorkers);
        bool Purge();

        // cooperative yield - allow us to switch to another fiber.
        void YieldFiber();

        // yield current fiber and keep doing so anytime we are rescheduled until the counter has reached zero
        void WaitForCounter(Counter* counter);
    }
}

#endif

