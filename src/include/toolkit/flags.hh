#pragma once
#ifndef INCLUDED_LPTOOLKIT_FLAGS_HH
#define INCLUDED_LPTOOLKIT_FLAGS_HH

#include <cstring>
#include "toolkit/compat.hh"
#include "toolkit/str.hh"

namespace lptk
{
    template<class FlagType, class PairType, size_t N>
    inline FlagType ParseFlagsFromPairs(const PairType(&flags)[N], const char* const spec)
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

    template<typename PairType, size_t N>
    inline lptk::Str StrFromFlags(const PairType(&pairs)[N], decltype(PairType::m_flags) flags)
    {
        lptk::DynAry<size_t> flagsToInclude;
        size_t bufferSize = 0;
        // make an assumption that combination flags are at the end, or at least specified AFTER
        // individual flags. This way we can serialize the group before the individual flags.
        for (size_t i = 0; i < N; ++i)
        {
            const auto j = N - 1 - i;
            const auto& curPair = pairs[j];
            if ((flags & curPair.m_flags) == curPair.m_flags)
            {
                flagsToInclude.push_back(j);
                flags = flags & ~curPair.m_flags;
                bufferSize += strlen(curPair.m_name);
            }
        }

        lptk::Str result{};

        if (flagsToInclude.empty())
            return result;

        lptk::DynAry<char> buffer;
        buffer.reserve(bufferSize + flagsToInclude.size() - 1);

        // output them in the same order as the enum is specified, for readability
        for (size_t i = 0; i < flagsToInclude.size(); ++i)
        {
            const auto j = flagsToInclude.size() - 1 - i;
            const auto& curPair = pairs[flagsToInclude[j]];
            const char* p = curPair.m_name;
            if (i > 0)
                buffer.push_back('+');

            while (*p)
                buffer.push_back(*p++);
        }

        result = buffer.data();
        return result;
    }

    template<class EnumType, class PairType, size_t N>
    inline EnumType ParseEnumFromPairs(const PairType(&pairs)[N], const char* const spec, EnumType def=EnumType(-1))
    {
        EnumType result{ def };
        if (spec == nullptr)
            return result;

        for (const auto& pair : pairs)
        {
            if (StrCaseEqual(pair.m_name, spec)) 
            {
                result = pair.m_value;
                break;
            }
        }
        return result;
    }
}

#endif
