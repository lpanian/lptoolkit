#include "toolkit/utf8str.hh"

namespace lptk
{
    size_t utf8_strlen(const char* sz)
    {
        if (!sz)
            return 0;
        size_t result = 0;
        auto cur = sz;

        while (*cur)
        {
            auto decode = utf8_decode(cur);
            cur = decode.second;
            ++result;
        }
        return result;
    }

    std::pair<CharPoint, const char*> utf8_decode(const char* sz)
    {
        if (!sz)
            return std::make_pair(0, nullptr);

        CharPoint result = 0;
        auto cur = unsigned char(*sz++);
        if (cur < 128)
        {
            result = CharPoint(cur);
        }
        else
        {
            CharPoint working = 0;
            auto numContinuation = 0;
            if ((cur & 0b1111'1000) == 0b1111'0000) // 4 bytes total
            {
                working = (cur & 0b0111);
                numContinuation = 3;
            }
            else if ((cur & 0b1111'0000) == 0b1110'0000) // 3 bytes total
            {
                working = (cur & 0b1111);
                numContinuation = 2;
            }
            else if (((cur & 0b1110'0000) == 0b1100'0000)) // 2 bytes total
            {
                working = (cur & 0b11111);
                numContinuation = 1;
            }

            while (numContinuation-- > 0)
            {
                working = working << 6;
                auto cont = unsigned char(*sz++);
                if ((cont & 0b1100'0000) != 0b1000'0000)
                {
                    // not a continuation byte, bail out.
                    working = 0;
                    break;
                }
                else
                {
                    working = working | CharPoint(cont & 0b0011'1111);
                }
            }

            result = working;
        }

        return std::make_pair(result, sz);
    }

}
