#pragma once

#include <cstdint>

namespace lptk
{
    inline char HexCharFromDigit(unsigned hexVal)
    {
        ASSERT(hexVal < 16);
        const char c = static_cast<char>(hexVal < 10 ?
            '0' + hexVal :
            'A' + (hexVal - 10));
        return c;
    }

    inline bool HexStrFromU64(char* buf, size_t bufLen, uint64_t val)
    {
        if (bufLen < 19)
            return false;
        buf[0] = '0';
        buf[1] = 'x';
        for (unsigned i = 0; i < 16; ++i)
        {
            const auto hexVal = uint8_t((val >> 4*i) & 0xF);
            const char c = HexCharFromDigit(hexVal);
            buf[(15-i) + 2] = c;
        }
        buf[18] = '\0';
        return true;
    }

    inline bool IsHexDigitChar(const char c)
    {
        return (c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'f') ||
            (c >= 'A' && c <= 'F');
    }

    inline unsigned UnsignedFromHexDigit(const char c)
    {
        unsigned result = 0;
        if (c >= '0' && c <= '9')
        {
            result = c - '0';
        }
        else if (c >= 'a' && c <= 'f')
        {
            result = (c - 'a') + 10;
        }
        else if (c >= 'A' && c <= 'F')
        {
            result = (c - 'A') + 10;
        }
        else
            result = ~unsigned(0);
        return result;
    }

    inline bool U64FromHexStr(uint64_t& val, const char* buf)
    {
        val = 0;
        if (buf[0] == '0' && buf[1] == 'x')
            buf += 2;

        unsigned i = 0;
        for (; i < 16 && buf[i]; ++i)
        {
            uint64_t valToAdd = 0;
            const char c = buf[i];
            valToAdd = UnsignedFromHexDigit(c);
            if (valToAdd >= 16)
                return false;
            val = (val << 4) | valToAdd;
        }
        if (buf[i] && IsHexDigitChar(buf[i]))
            return false;
        return true;
    }

}
