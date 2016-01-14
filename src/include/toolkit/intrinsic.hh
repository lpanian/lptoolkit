#pragma once
#ifndef INCLUDED_LPTOOLKIT_INTRINSIC_HH
#define INCLUDED_LPTOOLKIT_INTRINSIC_HH

#ifdef _MSC_VER
#include <intrin.h>
#endif

namespace lptk
{
#ifdef _MSC_VER
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

