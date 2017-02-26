#pragma once

#include <type_traits>

namespace lptk
{
    inline void SwapEndian(void* memory, size_t size)
    {
        char* bytes = reinterpret_cast<char*>(memory);
        for (size_t i = 0; i < size / 2; ++i)
        {
            char byte = bytes[i];
            bytes[i] = bytes[size - i - 1];
            bytes[size - i - 1] = byte;
        }
    }

    template< class T >
    inline void SwapEndian(T& value)
    {
        static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "cannot swap value");
        char* bytes = reinterpret_cast<char*>(&value);
        SwapEndian(bytes, sizeof(value));
    }

    union EndianHelper {
        constexpr EndianHelper(uint32_t val) : i(val) {}
        uint32_t i;
        char c[4];
    };

    constexpr bool IsNativeBigEndian()
    {
        return EndianHelper(0x01020304).c[0] == 0x01;
    }

    constexpr bool IsNativeLittleEndian()
    {
        return !IsNativeBigEndian();
    }

    template<class T>
    void NativeToLittleEndian(T& value)
    {
        if (IsNativeBigEndian())
            SwapEndian(value);
    }

    template<class T>
    void LittleToNativeEndian(T& value)
    {
        if (IsNativeBigEndian())
            SwapEndian(value);
    }

    template<class T>
    void NativeToBigEndian(T& value)
    {
        if (IsNativeLittleEndian())
            SwapEndian(value);
    }

    template<class T>
    void BigToNativeEndian(T& value)
    {
        if (IsNativeLittleEndian())
            SwapEndian(value);
    }
}
