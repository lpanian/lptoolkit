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
        Iter binSearch(Iter begin, Iter end, const T& val, Iter initialMid, Cmp&& cmp)
        {
            const auto count = end - begin;
            if(count < 1)
                return end;

            auto lo = begin;
            auto hi = end - 1;
            auto mid = initialMid;
        
            ASSERT(initialMid >= lo);
            ASSERT(initialMid <= hi);

            while(lo < hi)
            {
                const auto& midVal = *mid;
                if(!cmp(midVal, val))
                    hi = mid;
                else
                    lo = mid + 1;

                mid = lo + (hi - lo) / 2;
            }

            if(lo == hi && !cmp(*lo, val) && !cmp(val, *lo))
                return lo;
            else
                return end;
        }

        template<typename T, typename Iter, typename Cmp>
        Iter binSearchLower(Iter begin, Iter end, const T& val, Iter initialMid, Cmp&& cmp)
        {
            const auto count = end - begin;
            if(count < 1)
                return end;

            auto lo = begin;
            auto hi = end - 1;
            auto mid = initialMid;
        
            ASSERT(initialMid >= lo);
            ASSERT(initialMid <= hi);

            while(lo < hi)
            {
                const auto& midVal = *mid;
                if(!cmp(midVal, val))
                    hi = mid;
                else
                    lo = mid + 1;

                mid = lo + (hi - lo) / 2;
            }

            while (lo != begin && cmp(val, *lo)) 
                --lo;

            if(cmp(val, *lo))
                return end;
            return lo;
        }
    }

    template<typename T, typename Iter, typename Cmp = std::less<T>>
    Iter binSearch(Iter begin, Iter end, const T& val, Cmp&& cmp = Cmp()) 
    {
        const auto count = end - begin;
        return details::binSearch(begin, end, val, begin + count / 2, std::forward<Cmp>(cmp));
    }
    
    template<typename T, typename Iter, typename Cmp = std::less<T>>
    Iter binSearch(Iter begin, Iter end, const T& val, size_t mid, Cmp&& cmp = Cmp()) 
    {
        return details::binSearch(begin, end, val, begin + mid, std::forward<Cmp>(cmp));
    }

    template<typename T, typename Iter, typename Cmp = std::less<T>>
    Iter binSearchLower(Iter begin, Iter end, const T& val, Cmp&& cmp = Cmp()) 
    {
        const size_t count = end - begin;
        return details::binSearchLower(begin, end, val, begin + count / 2, std::forward<Cmp>(cmp));
    }
    
    template<typename T, typename Iter, typename Cmp = std::less<T>>
    Iter binSearchLower(Iter begin, Iter end, const T& val, size_t mid, Cmp&& cmp = Cmp()) 
    {
        return details::binSearchLower(begin, end, val, begin + mid, std::forward<Cmp>(cmp));
    }

}

#endif

