#include "toolkit/bitvector.hh"
#include <algorithm>

BitVector::BitVector(int initialSize, bool initialVal)
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

void BitVector::set(int index, bool value)
{
	int byteIndex = index >> 3;
	int bitIndex = index - byteIndex * 8;
	uint8_t byte = m_bytes[byteIndex];
	uint8_t mask = 1 << bitIndex;
	byte = (~mask & byte) | int(value) << bitIndex;
	m_bytes[byteIndex] = byte;
}

bool BitVector::get(int index) const
{
	int byteIndex = index >> 3;
	int bitIndex = index - byteIndex * 8;
	uint8_t byte = m_bytes[byteIndex];
	uint8_t mask = 1 << bitIndex;
	return 0 != (mask & byte);
}

void BitVector::resize(int newSize, bool value)
{
	const uint8_t newDataVal = value ? 0xFF : 0x00;
	const int numBytes = (newSize + 7) / 8;
	m_bytes.resize(numBytes, newDataVal);

	// copy the partial byte
	const int numWholeBits = 8 * (m_numBits / 8);
	const int numPartialBits = m_numBits - numWholeBits;
	const int numNewPartialBits = newSize - numWholeBits;
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
	if( (m_numBits+7) / 8 > (int)m_bytes.size())
		m_bytes.push_back(0);
	set(m_numBits - 1, value);
}

bool BitVector::operator[](int index) const
{
	return get(index);
}

void BitVector::swap(BitVector& other)
{
	std::swap(m_numBits, other.m_numBits);
	m_bytes.swap(other.m_bytes);
}

