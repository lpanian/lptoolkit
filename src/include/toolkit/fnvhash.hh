#pragma once

namespace lptk
{
    // algorithm from http://www.isthe.com/chongo/tech/comp/fnv/

    template<typename T>
    struct FNVParam {};

    template<> struct FNVParam<uint32_t> {
        static constexpr uint32_t offset = 2166136261;
        static constexpr uint32_t prime = 16777619;
    };
    template<> struct FNVParam<uint64_t> {
        static constexpr uint64_t offset = 14695981039346656037;
        static constexpr uint64_t prime = 1099511628211;
    };

    template<typename T>
    constexpr T fnv1a(const char* s,
        T hash = FNVParam<T>::offset,
        const T prime = FNVParam<T>::prime)
    {
        while (*s)
            hash = prime * (hash ^ *s++);
        return hash;
    }

    template<typename T>
    constexpr T fnv1(const char* s,
        T hash = FNVParam<T>::offset,
        const T prime = FNVParam<T>::prime)
    {
        while (*s)
            hash = (prime * hash) ^ *s++;
        return hash;
    }
}