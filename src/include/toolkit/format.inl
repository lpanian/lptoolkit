#pragma once
#ifndef INCLUDED_lptk_format_INL
#define INCLUDED_lptk_format_INL

#include <cstdarg>
#include <cstdio>
#include "toolkit/memarena.hh"
#include "toolkit/dynary.hh"

namespace lptk
{
    struct FormatContainer::ArgNode
    {
        char* m_str = nullptr;
        size_t m_len = 0;
    };

    template<size_t N>
    inline void SafeSnprintf(char(&buf)[N], const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
#if defined(LINUX)
        vsnprintf(buf, N - 1, fmt, args);
#elif defined(WINDOWS)
        _vsnprintf(buf, N - 1, fmt, args);
#endif
        va_end(args);
        buf[N - 1] = '\0';
    }

    struct FormatContainer::Data
    {
        Data(const char* const formatStr)
            : m_arena(1024)
        {
            const size_t len = strlen(formatStr) + 1;
            m_fmt = static_cast<char*>(m_arena.Alloc(len));
            std::strncpy(m_fmt, formatStr, len);
        }

        MemArena m_arena;
        char* m_fmt = nullptr;
        lptk::DynAry<ArgNode*> m_args;
    };

    inline FormatContainer::FormatContainer(const char* const formatStr)
        : m_data(make_unique<Data>(formatStr))
    {
    }
        
    inline FormatContainer::FormatContainer(FormatContainer&& other)
        : m_data(std::move(other.m_data))
    {
    }

    inline FormatContainer& FormatContainer::operator=(FormatContainer&& other)
    {
        m_data = std::move(other.m_data);
        return *this;
    }
        
    inline Str FormatContainer::GetStr() const
    {
        const char* const fmt = m_data->m_fmt;
        const auto& args = m_data->m_args;

        size_t lenFmt = 0;
        for(size_t i = 0; fmt[i] != '\0'; ++i)
        {
            if(fmt[i] == '%')
            {
                size_t j = i + 1;
                if(isdigit(fmt[j])) 
                {
                    int argIndex = atoi(&fmt[j]);
                    if(argIndex < 0) argIndex = 0;

                    // the length of these will be counted in arguments
                    while(isdigit(fmt[j])) 
                        ++j;

                    // an out of bounds index remains unformated.
                    if(static_cast<size_t>(argIndex) >= args.size())
                    {
                        lenFmt += j - i;
                    }
                    else
                    {   
                        lenFmt += args[argIndex]->m_len - 1;
                    }
                    i = j-1;
                }
                else if(fmt[j] == '%')
                {
                    lenFmt += 1;
                    i = j;
                }
                else
                {
                    lenFmt += 2;
                    i = j;
                }
            }
            else
                ++lenFmt;
        }

        MemArena localArena(sizeof(char) * (lenFmt + 1));
        char* strData = static_cast<char*>(localArena.Alloc(sizeof(char) * (lenFmt + 1)));
        memset(strData, 0, sizeof(char) * (lenFmt + 1));

        size_t resultIdx = 0;
        for(size_t i = 0; fmt[i] != '\0'; ++i)
        {
            if(fmt[i] == '%')
            {
                size_t j = i + 1;
                if(isdigit(fmt[j])) 
                {
                    int argIndex = atoi(&fmt[j]);
                    if(argIndex < 0) argIndex = 0;

                    if(static_cast<size_t>(argIndex) < args.size())
                    {
                        strncpy(&strData[resultIdx], args[argIndex]->m_str, args[argIndex]->m_len - 1);
                        resultIdx += args[argIndex]->m_len - 1;

                        while(isdigit(fmt[j])) 
                            ++j;
                        i = j-1;
                    }
                    else
                    {
                        strData[resultIdx++] = '%';
                        while(isdigit(fmt[j])) 
                            strData[resultIdx++] = fmt[j++];
                        i = j-1;
                    }
                }
                else if(fmt[j] == '%')
                {
                    strData[resultIdx++] = '%';
                    i = j;
                }
                else
                {
                    // skip the 2nd digit just in case the string was "%%"
                    strData[resultIdx++] = fmt[i++];
                    strData[resultIdx++] = fmt[i];
                }
            }
            else
                strData[resultIdx++] = fmt[i];
        }

        ASSERT(resultIdx <= lenFmt);
        strData[resultIdx] = '\0';
        Str result(strData, resultIdx);
        return result;
    }
        
    inline FormatContainer::ArgNode* FormatContainer::AllocNode(size_t const len)
    {
        ArgNode* node = m_data->m_arena.Alloc<ArgNode>();
        m_data->m_args.push_back(node);
        
        node->m_str = static_cast<char*>(m_data->m_arena.Alloc(len));
        node->m_str[0] = '\0';
        node->m_len = len;

        return node;
    }
        
    inline FormatContainer FormatContainer::With(char const* const s)
    {
        const size_t len = strlen(s) + 1;
        auto node = AllocNode(len);
        strncpy(node->m_str, s, len);
        return std::move(*this);
    }
        
    inline FormatContainer FormatContainer::With(Str const& s)
    {
        return With(s.c_str());
    }

    inline FormatContainer FormatContainer::With(uint32_t const i)
    {
        char buf[64];
        SafeSnprintf(buf, "%" PRIu32, i);
        buf[ARRAY_SIZE(buf)-1] = '\0';
        return With(buf);
    }

    inline FormatContainer FormatContainer::With(uint64_t const i)
    {
        char buf[64];
        SafeSnprintf(buf, "%" PRIu64, i);
        buf[ARRAY_SIZE(buf)-1] = '\0';
        return With(buf);
    }

    inline FormatContainer FormatContainer::With(int32_t const i)
    {
        char buf[64];
        SafeSnprintf(buf, "%" PRId32, i);
        buf[ARRAY_SIZE(buf)-1] = '\0';
        return With(buf);
    }

    inline FormatContainer FormatContainer::With(int64_t const i)
    {
        char buf[64];
        SafeSnprintf(buf, "%" PRId64, i);
        buf[ARRAY_SIZE(buf)-1] = '\0';
        return With(buf);
    }

    inline FormatContainer FormatContainer::With(float const f)
    {
        return With(static_cast<double>(f));
    }

    inline FormatContainer FormatContainer::With(double const f)
    {
        char buf[64];
        SafeSnprintf(buf, "%f", f);
        buf[ARRAY_SIZE(buf)-1] = '\0';
        return With(buf);
    }

    inline FormatContainer Format(const char* const formatStr)
    {
        return formatStr;
    }

}

#endif

