#pragma once
#ifndef INCLUDED_toolkit_bitvector_HH
#define INCLUDED_toolkit_bitvector_HH

#include <cstdint>
#include <functional>

#include "dynary.hh"

namespace lptk 
{
    ////////////////////////////////////////////////////////////////////////////////
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
        size_t find_next_true(size_t after) const;
        size_t find_first_false() const;
        size_t find_next_false(size_t after) const;

        void unset_all();

        void subtract(const BitVector& other);
        void add(const BitVector& other);
    private:
        size_t m_numBits;
        DynAry<PrimType> m_bytes;
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    class BitVectorEnumerator
    {
        const BitVector& m_bv;
    public:
        BitVectorEnumerator(const BitVector& bv) : m_bv(bv) {}

        class iterator
        {
            ::std::reference_wrapper<const BitVector> m_bv;
            size_t m_index = 0;
        public:
            iterator(::std::reference_wrapper<const BitVector> bv, size_t index) : m_bv(bv), m_index(index) {}

            iterator& operator++() {
                m_index = m_bv.get().find_next_true(m_index + 1);
                return *this;
            }
            
            iterator operator++(int) {
                iterator self(m_bv, m_index);
                m_index = m_bv.get().find_next_true(m_index + 1);
                return self;
            }

            size_t operator*() const { return m_index; }

            bool operator==(const iterator& o) const {
                return &o.m_bv.get() == &m_bv.get() && o.m_index == m_index;
            }
            bool operator!=(const iterator& o) const {
                return !operator==(o);
            }
        };

        iterator begin() const { return iterator(m_bv, 0); }
        iterator end() const { return iterator(m_bv, m_bv.size()); }
    };

    inline BitVectorEnumerator enumerate(const BitVector& bv) {
        return BitVectorEnumerator(::std::cref(bv));
    }
}

#endif

