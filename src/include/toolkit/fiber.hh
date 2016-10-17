#pragma once
#ifndef INCLUDED_LPTK_FIBER_HH
#define INCLUDED_LPTK_FIBER_HH

#include <atomic>

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
        public:
            Fiber(Counter* counter, FiberFunc fn, void* userData);

            Fiber(const Fiber&) = delete;
            Fiber& operator=(const Fiber&) = delete;
            Fiber(Fiber&& other) = delete;
            Fiber& operator=(Fiber&& other) = delete;

            ~Fiber();
        protected:
            void Run();

            Counter* m_counter = nullptr;
            FiberFunc m_fn = nullptr;
            void* m_userData = nullptr;
            Fiber* m_next = nullptr;

#if defined(WINDOWS)
            static void CALLBACK FiberMain(void* param);
            // actual implementation pointer (void* on windows)
            void* m_fiber = nullptr;
#elif defined(LINUX)
#error LinuxFiber implementation not yet implemented. You should do that!
#endif
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

