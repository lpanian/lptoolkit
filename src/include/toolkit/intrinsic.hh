#pragma once
#ifndef INCLUDED_LPTOOLKIT_INTRINSIC_HH
#define INCLUDED_LPTOOLKIT_INTRINSIC_HH

#ifdef _WINDOWS
#include <intrin.h>
#endif

namespace lptk
{
#ifdef _WINDOWS
    inline unsigned int PopCount(unsigned int value)
    {
        return __popcnt(value);
    }

    inline unsigned int FindFirstSet(unsigned int value)
    {
        unsigned long index;
        if (_BitScanForward(&index, value))
        {
            return index + 1;
        }
        return 0;
    }

#endif

#ifdef __GNUC__
    inline unsigned int PopCount(unsigned int value)
    {
        return __builtin_popcount(value);
    }

    inline unsigned int FindFirstSet(unsigned int value)
    {
        return __builtin_ffs(value);
    }
#endif
}

#endif

