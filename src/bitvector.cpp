#include "toolkit/bitvector.hh"
#include <algorithm>

namespace lptk
{

BitVector::BitVector(size_t initialSize, bool initialVal)
	: m_numBits(initialSize)
	, m_bytes((initialSize+7) / 8, uint8_t(initialVal ? 0xFF : 0))
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
	m_numBits = o.m_numBits;
	o.m_numBits = 0;
	m_bytes = std::move(o.m_bytes);
	return *this;
}
	
BitVector& BitVector::operator=(const BitVector& o)
{
	if(this != &o)
	{
		m_numBits = o.m_numBits;
		m_bytes = o.m_bytes;
	}
	return *this;
}

void BitVector::set(size_t index, bool value)
{
	auto const byteIndex = index >> 3;
	auto const bitIndex = index - byteIndex * 8;
	auto const byte = m_bytes[byteIndex];
	uint8_t const mask = 1 << bitIndex;
	auto const masked = static_cast<uint8_t>((~mask & byte) | (uint8_t(value) << bitIndex));
	m_bytes[byteIndex] = masked;
}

bool BitVector::get(size_t index) const
{
	auto const byteIndex = index >> 3;
	auto const bitIndex = index - byteIndex * 8;
	auto const byte = m_bytes[byteIndex];
	uint8_t const mask = 1 << bitIndex;
	return 0 != (mask & byte);
}

void BitVector::resize(size_t newSize, bool value)
{
	const uint8_t newDataVal = value ? 0xFF : 0x00;
	const auto numBytes = (newSize + 7) / 8;
	m_bytes.resize(numBytes, newDataVal);

	// copy the partial byte
	const auto numWholeBits = 8 * (m_numBits / 8);
	const auto numPartialBits = m_numBits - numWholeBits;
	const auto numNewPartialBits = newSize - numWholeBits;
	if(numPartialBits > 0) 
	{
		const uint8_t partialByte = m_bytes[numBytes - 1];
		const uint8_t oldMask = (1 << numPartialBits) - 1;
		// extra mask with 0xff to get rid of VS exception about losing data. It claims
		// the resulting code won't be changed...
		const uint8_t newMask = 0xFF & (~oldMask & ((1 << numNewPartialBits) - 1)); 
		const uint8_t newByte = (partialByte & oldMask) | (newDataVal & newMask);
		m_bytes[numBytes - 1] = newByte;
	}

	m_numBits = newSize;
}

void BitVector::push_back(bool value)
{
	++m_numBits;
	if( (m_numBits+7) / 8 > m_bytes.size())
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


