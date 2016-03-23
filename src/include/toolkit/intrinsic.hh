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

#endif

#ifdef __GNUC__
    inline unsigned int PopCount(unsigned int value)
    {
        return __builtin_popcount(value);
    }
#endif
}

#endif

