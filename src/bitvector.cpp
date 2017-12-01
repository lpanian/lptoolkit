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
        ASSERT(byteIndex < m_bytes.size());
        auto const bitIndex = index - byteIndex * kNumBits;
        auto const byte = m_bytes[byteIndex];
        PrimType const mask = PrimType(1) << bitIndex;
        auto const masked = static_cast<PrimType>((~mask & byte) | (PrimType(value) << bitIndex));
        m_bytes[byteIndex] = masked;
    }

    bool BitVector::get(size_t index) const
    {
        auto const byteIndex = index >> kLog2;
        ASSERT(byteIndex < m_bytes.size());
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
            const PrimType oldMask = (PrimType(1) << numPartialBits) - 1;
            // extra mask with 0xff to get rid of VS exception about losing data. It claims
            // the resulting code won't be changed...
            const PrimType newMask = kMask & (~oldMask & ((PrimType(1) << numNewPartialBits) - 1));
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
        
    size_t BitVector::pop_count() const
    {
        size_t result = 0;
        for (auto&& block : m_bytes)
            // ugh, make this generic?
            result += lptk::PopCount64(block);
        return result;
    }

    size_t BitVector::find_first_true() const
    {
        return find_next_true(0);
    }
        
    size_t BitVector::find_next_true(size_t first) const
    {
        if (first >= m_numBits || !m_numBits)
            return m_numBits;
        const size_t firstBlockIndex = (first >> kLog2);
        auto index = firstBlockIndex * kNumBits;

        auto bitIndex = first - index;
        const auto firstMask = ~((PrimType(1) << bitIndex) - 1);
        const auto nextIndex = lptk::FirstBitIndex64(firstMask & m_bytes[0]);
        index += nextIndex;
        if (nextIndex < kNumBits)
            return index;

        for (auto i = firstBlockIndex + 1; i < m_bytes.size(); ++i)
        {
            const auto blockIndex = lptk::FirstBitIndex64(m_bytes[i]);
            index += blockIndex;
            if (blockIndex < kNumBits)
                return index;
        }
        return m_numBits;
    }
    
    size_t BitVector::find_first_false() const
    {
        return find_next_false(0);
    }
    
    size_t BitVector::find_next_false(size_t first) const
    {
        if (first >= m_numBits || !m_numBits)
            return m_numBits;
        const size_t firstBlockIndex = (first >> kLog2);
        auto index = firstBlockIndex * kNumBits;

        auto bitIndex = first - index;
        const auto firstMask = ~((PrimType(1) << bitIndex) - 1);
        const auto nextIndex = lptk::FirstBitIndex64(firstMask & ~(m_bytes[0]));
        index += nextIndex;
        if (nextIndex < kNumBits)
            return index;

        for (auto i = firstBlockIndex + 1; i < m_bytes.size(); ++i)
        {
            const auto blockIndex = lptk::FirstBitIndex64(~m_bytes[i]);
            index += blockIndex;
            if (blockIndex < kNumBits)
                return index;
        }
        return m_numBits;
    }
        
    void BitVector::unset_all()
    { 
        for (auto&& block : m_bytes)
            block = PrimType(0);
    }
        
    void BitVector::subtract(const BitVector& other)
    {
        auto bitsRemaining = m_numBits;
        auto otherBitsRemaining = other.m_numBits;
        // assume any past-numBits values in other are 0 and do nothing
        const auto maxCount = lptk::Min(m_bytes.size(), other.m_bytes.size());
        for (size_t i = 0; i < maxCount; ++i)
        {
            const auto bitShift = bitsRemaining & (kNumBits - 1);
            const auto otherBitShift = otherBitsRemaining & (kNumBits - 1);

            auto bitMask = ~PrimType(0) >> bitShift;
            auto otherBitMask = ~PrimType(0) >> otherBitShift;

            m_bytes[i] = (m_bytes[i] & bitMask) & ~(other.m_bytes[i] & otherBitMask);

            bitsRemaining -= kNumBits;
            otherBitsRemaining -= other.kNumBits;
        }
    }

    void BitVector::add(const BitVector& other)
    {
        if (other.size() > size())
        {
            resize(other.size());
        }
       
        auto bitsRemaining = m_numBits;
        // assume any past-numBits values in other are 0 and do nothing
        const auto maxCount = lptk::Min(m_bytes.size(), other.m_bytes.size());
        for (size_t i = 0; i < maxCount; ++i)
        {
            const auto bitShift = bitsRemaining & (kNumBits - 1);

            auto bitMask = ~PrimType(0) >> bitShift;

            m_bytes[i] = (m_bytes[i] & bitMask) | (other.m_bytes[i] & bitMask);

            bitsRemaining -= kNumBits;
        }
    }
}


