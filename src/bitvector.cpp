#include "toolkit/bitvector.hh"

#include <algorithm>

namespace lptk
{
    BitVector::BitVector(size_t initialSize, bool initialVal)
        : m_numBits(initialSize)
        , m_bytes((initialSize + kNumBits - 1) / kNumBits, PrimType(initialVal ? kMask : 0))
    {
    }

    BitVector::BitVector(BitVector&& o)
        : m_numBits(o.m_numBits)
        , m_bytes(std::move(o.m_bytes))
    {
        o.m_numBits = 0;
    }

    BitVector::BitVector(const BitVector& o)
        : m_numBits(o.m_numBits)
        , m_bytes(o.m_bytes)
    {
    }

    BitVector& BitVector::operator=(BitVector&& o)
    {
        if (this != &o)
        {
            m_numBits = o.m_numBits;
            o.m_numBits = 0;
            m_bytes = std::move(o.m_bytes);
        }
        return *this;
    }

    BitVector& BitVector::operator=(const BitVector& o)
    {
        if (this != &o)
        {
            m_numBits = o.m_numBits;
            m_bytes = o.m_bytes;
        }
        return *this;
    }

    void BitVector::set(size_t index, bool value)
    {
        auto const byteIndex = index >> kLog2;
        auto const bitIndex = index - byteIndex * kNumBits;
        auto const byte = m_bytes[byteIndex];
        PrimType const mask = PrimType(1) << bitIndex;
        auto const masked = static_cast<PrimType>((~mask & byte) | (PrimType(value) << bitIndex));
        m_bytes[byteIndex] = masked;
    }

    bool BitVector::get(size_t index) const
    {
        auto const byteIndex = index >> kLog2;
        auto const bitIndex = index - byteIndex * kNumBits;
        auto const byte = m_bytes[byteIndex];
        PrimType const mask = PrimType(1) << bitIndex;
        return 0 != (mask & byte);
    }

    void BitVector::resize(size_t newSize, bool value)
    {
        const PrimType newDataVal = value ? kMask : 0x00;
        const auto numBytes = (newSize + kNumBits - 1) / kNumBits;
        m_bytes.resize(numBytes, newDataVal);

        // copy the partial byte
        const auto numWholeBits = kNumBits * (m_numBits / kNumBits);
        const auto numPartialBits = m_numBits - numWholeBits;
        const auto numNewPartialBits = newSize - numWholeBits;
        if (numPartialBits > 0)
        {
            const PrimType partialByte = m_bytes[numBytes - 1];
            const PrimType oldMask = (1 << numPartialBits) - 1;
            // extra mask with 0xff to get rid of VS exception about losing data. It claims
            // the resulting code won't be changed...
            const PrimType newMask = kMask & (~oldMask & ((1 << numNewPartialBits) - 1));
            const PrimType newByte = (partialByte & oldMask) | (newDataVal & newMask);
            m_bytes[numBytes - 1] = newByte;
        }

        m_numBits = newSize;
    }

    void BitVector::push_back(bool value)
    {
        ++m_numBits;
        if ((m_numBits + kNumBits - 1) / kNumBits > m_bytes.size())
            m_bytes.push_back(0);
        set(m_numBits - 1, value);
    }

    bool BitVector::operator[](size_t index) const
    {
        return get(index);
    }

    void BitVector::swap(BitVector& other)
    {
        std::swap(m_numBits, other.m_numBits);
        m_bytes.swap(other.m_bytes);
    }
}


