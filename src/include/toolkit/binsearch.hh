#pragma once
#ifndef INCLUDED_LPTK_BINSEARCH_HH
#define INCLUDED_LPTK_BINSEARCH_HH

#include <cstdint>
#include <functional>

namespace lptk
{
    namespace details
    {
        template<typename T, typename Iter, typename Cmp>
        Iter binSearch(Iter begin, Iter end, const T& val, Cmp&& cmp)
        {
            const auto it = binSearchLowerBound(begin, end, val, std::forward<Cmp>(cmp));
            if (it != end && !cmp(val, *it))
                return it;
            return end;
        }

        template<typename T, typename Iter, typename Cmp>
        Iter binSearchLowerBound(Iter begin, Iter end, const T& val, Cmp&& cmp)
        {
            auto count = std::distance(begin, end);
            while (count > 0)
            {
                auto it = begin;
                const auto midStep = count / 2;
                std::advance(it, midStep);
                if (cmp(*it, val))
                {
                    begin = ++it;
                    count -= midStep + 1;
                }
                else
                {
                    count = midStep;
                }
            }
            return begin;
        }
    }

    template<typename T, typename Iter, typename Cmp = std::less<T>>
    Iter binSearch(Iter begin, Iter end, const T& val, Cmp&& cmp = Cmp()) 
    {
        return details::binSearch(begin, end, val, std::forward<Cmp>(cmp));
    }

    template<typename T, typename Iter, typename Cmp = std::less<T>>
    Iter binSearchLowerBound(Iter begin, Iter end, const T& val, Cmp&& cmp = Cmp()) 
    {
        return details::binSearchLowerBound(begin, end, val, std::forward<Cmp>(cmp));
    }

}

#endif

