#pragma once
#ifndef INCLUDED_lptoolkit_parallel_HH
#define INCLUDED_lptoolkit_parallel_HH

#include <cstdint>

#ifdef WINDOWS
#include <intrin.h>
#pragma intrinsic(_InterlockedExchangeAdd)
#pragma intrinsic(_InterlockedCompareExchange)
#endif

namespace lptk
{

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
