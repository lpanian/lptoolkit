#pragma once
#ifndef INCLUDED_toolkit_bitvector_HH
#define INCLUDED_toolkit_bitvector_HH

#include <cstdint>

#include "dynary.hh"

namespace lptk 
{

    class BitVector
    {
        using PrimType = uint64_t;
        static constexpr auto kNumBits = sizeof(PrimType) * 8;
        static constexpr auto kLog2 = 6;
        static constexpr auto kMask = ~PrimType(0);

    public:
        BitVector(size_t initialSize = 0, bool initialVal = false);
        BitVector(BitVector&& o);
        BitVector(const BitVector& o);
        BitVector& operator=(const BitVector& o);
        BitVector& operator=(BitVector&& o);

        void set(size_t index, bool value);
        bool get(size_t index) const;

        size_t size() const { return m_numBits; }
        void resize(size_t newSize, bool value = false);

        void push_back(bool value);

        bool operator[](size_t index) const;
        void swap(BitVector& other);

        size_t pop_count() const;
        size_t find_first_true() const;
        size_t find_first_false() const;

        void unset_all();

        void subtract(const BitVector& other);
        void add(const BitVector& other);
    private:
        size_t m_numBits;
        DynAry<PrimType> m_bytes;
    };

}

#endif

