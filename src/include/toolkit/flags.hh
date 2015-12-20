#pragma once
#ifndef INCLUDED_LPTOOLKIT_FLAGS_HH
#define INCLUDED_LPTOOLKIT_FLAGS_HH

#include <cstring>
#include "toolkit/compat.hh"

namespace lptk
{
    template<class FlagType, class PairType, size_t N>
    FlagType ParseFlagsFromPairs(const PairType(&flags)[N], const char* const spec)
    {
        FlagType result = 0;
        if (spec == nullptr)
            return result;

        auto cur = spec;
        while (*cur)
        {
            while (*cur && (isspace(*cur) || *cur == '+')) 
                ++cur;
            auto end = cur;
            while (*end && !(isspace(*end) || *end == '+')) 
                ++end;

            const auto len = static_cast<unsigned int>(end - cur);
            if ( len > 0 )
            {
                for (const auto& pair : flags)
                {
                    if (StrNCaseEqual(pair.m_name, cur, len))
                    {
                        result = result | pair.m_flags;
                        break;
                    }
                }
            }
            cur = end;
        }

        return result;
    }
}

#endif
