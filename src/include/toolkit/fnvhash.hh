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
            hash = T(static_cast<unsigned long long>(prime) * (hash ^ *s++));
        return hash;
    }
    
    template<typename T>
    constexpr T fnv1a_n(const char* s, size_t len,
        T hash = FNVParam<T>::offset,
        const T prime = FNVParam<T>::prime)
    {
        while (len-- > 0)
            hash = T(static_cast<unsigned long long>(prime) * (hash ^ *s++));
        return hash;
    }

    template<typename T>
    constexpr T fnv1(const char* s,
        T hash = FNVParam<T>::offset,
        const T prime = FNVParam<T>::prime)
    {
        while (*s)
            hash = T(static_cast<unsigned long long>(prime) * hash) ^ *s++;
        return hash;
    }
    
    template<typename T>
    constexpr T fnv1_n(const char* s, size_t len,
        T hash = FNVParam<T>::offset,
        const T prime = FNVParam<T>::prime)
    {
        while (len-- > 0)
            hash = T(static_cast<unsigned long long>(prime) * hash) ^ *s++;
        return hash;
    }
    
    template<typename T, typename V>
    constexpr T fnv1(const V& v, 
        T hash = FNVParam<T>::offset,
        const T prime = FNVParam<T>::prime)
    {
        return fnv1_n(reinterpret_cast<const char*>(&v), sizeof(v), hash, prime);
    }

    template<typename T, typename V>
    constexpr T fnv1a(const V& v, 
        T hash = FNVParam<T>::offset,
        const T prime = FNVParam<T>::prime)
    {
        return fnv1a_n(reinterpret_cast<const char*>(&v), sizeof(v), hash, prime);
    }
}