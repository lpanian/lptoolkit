#pragma once
#ifndef INCLUDED_LPTOOLKIT_PARALLEL_HH
#define INCLUDED_LPTOOLKIT_PARALLEL_HH

#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    class Spinlock
    {
    public:
        void lock()
        {
            while (m_lock.exchange(true, std::memory_order_acq_rel)) {}
        }
        void unlock()
        {
            m_lock.store(false, std::memory_order_release);
        }
    private:
        static constexpr unsigned kCacheLine = 64;
        std::atomic<bool> m_lock = false;
        char padding[kCacheLine - sizeof(decltype(m_lock))];

    };

    ////////////////////////////////////////////////////////////////////////////////
    // 'Semaphore' using condition variables.
    class Semaphore
    {
    public:
        Semaphore() = default;
        ~Semaphore() = default;

        Semaphore(const Semaphore&) = default;
        Semaphore& operator=(const Semaphore&) = default;
        
        Semaphore(Semaphore&&) = default;
        Semaphore& operator=(Semaphore&&) = default;

        void Acquire(unsigned long val=1);
        void Release(unsigned long val=1);
        unsigned long GetCount();
    private:
        std::condition_variable m_cv;
        std::mutex m_mutex;
        unsigned long m_count = 0;
    };
}

#endif
