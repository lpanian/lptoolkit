#pragma once
#ifndef INCLUDED_LPTOOLKIT_PARALLEL_HH
#define INCLUDED_LPTOOLKIT_PARALLEL_HH

#include <cstdint>
#include <condition_variable>
#include <mutex>
#include <atomic>

#ifdef WINDOWS
#include <intrin.h>
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#endif

namespace lptk
{
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
    private:
        std::condition_variable m_cv;
        std::mutex m_mutex;
        unsigned long m_count = 0;
    };


    ////////////////////////////////////////////////////////////////////////////////
    // TODO: Replace these functions with std::atomic stuff where it's used, and delete these.

template<class T>
inline bool AtomicCompareAndSwap(volatile T* target, T oldval, T newval)
{
    (void)oldval; // for some reason this gives a 'set but not used' warning, this is to suppress that.
#if defined(LINUX)
    return __sync_bool_compare_and_swap(target, oldval, newval);
#elif defined(WINDOWS)
    return oldval == _InterlockedCompareExchange(target, newval, oldval);
#else
#error "AtomicCompareAndSwap is missing an implementation for this platform."
    return false;
#endif
    
}


template<class T>
inline T FetchAndAdd(volatile T* target, T amount) 
{
#if defined(LINUX)
    return __sync_fetch_and_add(target, amount); 
#elif defined(WINDOWS)
    return _InterlockedExchangeAdd(target, amount);
#else
#error "FetchAndAdd is missing an implementation for this platform."
    return 0.f;
#endif
}

inline float FetchAndAdd(volatile float* target, float amount)
{
    static_assert(sizeof(float) == sizeof(uint32_t), "Sizeof float is not the same as uint32_t");

    union f_to_i {
        float f;
        uint32_t i;
    };



    f_to_i oldval;
    f_to_i newval; 
    do {
        oldval.f = *target;
        newval.f = oldval.f + amount;
    } while(!AtomicCompareAndSwap(reinterpret_cast<volatile uint32_t*>(target), oldval.i, newval.i));
    return oldval.f;
}

}

#endif
