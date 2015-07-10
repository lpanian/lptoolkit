#pragma once
#ifndef INCLUDED_lptk_binsearch_HH
#define INCLUDED_lptk_binsearch_HH

#include <cstdint>

namespace lptk
{
    namespace details
    {
        template<typename T, typename Iter>
            int64_t binSearch(const Iter& begin, const Iter& end, const T& val, size_t initialMid)
        {
            const size_t count = end - begin;
            if(count < 1)
                return 0;

            size_t lo = 0,
                hi = count - 1,
                mid = initialMid;
        
            ASSERT(initialMid >= lo);
            ASSERT(initialMid <= hi);

            while(lo < hi)
            {
                const T& midVal = *(begin + mid);
                if(val <= midVal)
                    hi = mid;
                else
                    lo = mid + 1;

                mid = lo + (hi - lo) / 2;
            }

            if(lo == hi && *(begin + lo) == val)
                return lo;
            else
                return -1;
        }
        
        template<typename T, typename Iter>
            int64_t binSearchLower(const Iter& begin, const Iter& end, const T& val, size_t initialMid)
        {
            const size_t count = end - begin;
            if(count < 1)
                return 0;

            size_t lo = 0,
                hi = count - 1,
                mid = initialMid;
        
            ASSERT(initialMid >= lo);
            ASSERT(initialMid <= hi);

            while(lo < hi)
            {
                const T& midVal = *(begin + mid);
                if(val <= midVal)
                    hi = mid;
                else
                    lo = mid + 1;

                mid = lo + (hi - lo) / 2;
            }

            while(lo > 0 && *(begin + lo) > val)
                --lo;

            if(*(begin + lo) > val)
                return -1;
            return lo;
        }
    }

    template<typename T, typename Iter>
    int64_t binSearch(const Iter& begin, const Iter& end, const T& val) 
    {
        const size_t count = end - begin;
        return details::binSearch(begin, end, val, count / 2);
    }
    
    template<typename T, typename Iter>
    int64_t binSearch(const Iter& begin, const Iter& end, const T& val, size_t mid) 
    {
        return details::binSearch(begin, end, val, mid);
    }

    template<typename T, typename Iter>
    int64_t binSearchLower(const Iter& begin, const Iter& end, const T& val) 
    {
        const size_t count = end - begin;
        return details::binSearchLower(begin, end, val, count / 2);
    }
    
    template<typename T, typename Iter>
    int64_t binSearchLower(const Iter& begin, const Iter& end, const T& val, size_t mid) 
    {
        return details::binSearchLower(begin, end, val, mid);
    }

}

#endif

