#pragma once

#include <cstdint>

namespace lptk
{
    enum class FourCC : uint32_t {};
    constexpr FourCC FourCCFromStr(const char* s)
    {
        return static_cast<FourCC>(
            static_cast<uint32_t>(s[0]) |
            static_cast<uint32_t>(s[1] << 8) |
            static_cast<uint32_t>(s[2] << 16) |
            static_cast<uint32_t>(s[3] << 24)
            );
    }
    namespace literals {
        constexpr FourCC operator "" _4cc(const char* s, size_t) { return FourCCFromStr(s); }
    }

    inline void StringFromFourCC(char(&buf)[5], FourCC code)
    {
        auto codeint = static_cast<uint32_t>(code);
        buf[0] = static_cast<char>(codeint & 0xff); codeint >>= 8;
        buf[1] = static_cast<char>(codeint & 0xff); codeint >>= 8;
        buf[2] = static_cast<char>(codeint & 0xff); codeint >>= 8;
        buf[3] = static_cast<char>(codeint & 0xff); 
        buf[4] = '\0';
    }
}

