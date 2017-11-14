#pragma once
#ifndef INCLUDED_toolkit_range_HH
#define INCLUDED_toolkit_range_HH

namespace lptk
{
    template<typename T>
    class Range
    {
        T m_start;
        T m_end;
    public:
        Range() : m_start(T()), m_end(T()) {}
        Range(T start, T end) 
            : m_start(start), m_end(end)
        {
        }

        T Start() const { return m_start; }
        T End() const { return m_end; }

        T Length() const {
            return m_start < m_end ? m_end - m_start : 0;
        }

        void Set(T start, T end) 
        {
            m_start = start;
            m_end = end;
        }
    };
}

#endif
