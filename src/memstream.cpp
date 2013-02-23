#include <cstring>
#include <type_traits>
#include "toolkit/common.hh"
#include "toolkit/memstream.hh"

////////////////////////////////////////////////////////////////////////////////
inline void SwapEndian(void* memory, int size)
{
	char* bytes = reinterpret_cast<char*>(memory);
	for(int i = 0; i < size/2; ++i)
	{
		char byte = bytes[i];
		bytes[i] = bytes[size - i - 1];
		bytes[size - i - 1] = byte;
	}
}

template< class T >
inline void SwapEndian(T& value)
{
	ASSERT(std::is_arithmetic<T>::value || std::is_enum<T>::value);
	char* bytes = reinterpret_cast<char*>(&value);
	SwapEndian(bytes, sizeof(value));
}

////////////////////////////////////////////////////////////////////////////////
MemWriter::MemWriter() 
	: m_b(0), m_s(0), m_pos(0), m_error(true), m_flags(0)
{}

MemWriter::MemWriter(char* buffer, size_t size, int flags) 
	: m_b(buffer),m_s(size),m_pos(0),m_error(false),
	 m_flags(flags)
{}
	
void MemWriter::Put(const void* bytes, size_t size, bool swapEndian) 
{
	using namespace MemFormatFlag;
	if(m_s - m_pos >= size) {
		memcpy(&m_b[m_pos],bytes,size);
		if(swapEndian)
		{
			// native is little endian and data is big
			if(IsNativeLittleEndian() && (m_flags & FLAG_BigEndianData) != 0)
				SwapEndian(&m_b[m_pos], size);
			// else if native is big and data is little
			else if(IsNativeBigEndian() && (m_flags & FLAG_LittleEndianData) != 0)
				SwapEndian(&m_b[m_pos], size);
		}
		m_pos += size;
	}
	else
		m_error = true;
}

void MemWriter::Advance(size_t size) 
{
	if(m_s - m_pos >= size) {
		m_pos += size;
	} 
	else
		m_error = true;
}

void MemWriter::AlignedAdvance(size_t align)
{
	const size_t requiredAlignMask = align - 1;
	const size_t alignedPos = (m_pos + requiredAlignMask) & ~requiredAlignMask;
	const size_t padding = alignedPos - m_pos;
	if(padding > 0)
		Advance(padding);
}
	
void MemWriter::PutColor(const Color& c, bool swapEndian)
{
	Put<float>(c.r, swapEndian);
	Put<float>(c.g, swapEndian);
	Put<float>(c.b, swapEndian);
	Put<float>(c.a, swapEndian);
}

void MemWriter::PutColorRGBA(ColorRGBA c)
{
	Put<uint8_t>(c.r, false);
	Put<uint8_t>(c.g, false);
	Put<uint8_t>(c.b, false);
	Put<uint8_t>(c.a, false);
}

////////////////////////////////////////////////////////////////////////////////
MemReader::MemReader() 
	: m_b(0), m_s(0), m_pos(0), m_error(true) 
{}

MemReader::MemReader(const char* buffer, size_t size, int flags) 
	: m_b(buffer),m_s(size),m_pos(0),m_error(false), m_flags(flags) 
{}

void MemReader::Get(void* bytes, size_t size, bool swapEndian) 
{
	using namespace MemFormatFlag;
	if( (m_s - m_pos) >= size) 
	{
		memcpy(bytes,&m_b[m_pos],size);
		if(swapEndian)
		{
			// native is little endian and data is big
			if(IsNativeLittleEndian() && (m_flags & FLAG_BigEndianData) != 0)
				SwapEndian(bytes, size);
			// else if native is big and data is little
			else if(IsNativeBigEndian() && (m_flags & FLAG_LittleEndianData) != 0)
				SwapEndian(bytes, size);
		}
		m_pos += size;
	}
	else
		m_error = true;
}

void MemReader::Consume(size_t size) 
{
	if( (m_s - m_pos) >= size) {
		m_pos += size;
	}
	else
		m_error = true;
}

void MemReader::AlignedConsume(size_t align) 
{
	size_t requiredAlignMask = align - 1;
	size_t alignedPos = (m_pos + requiredAlignMask) & ~requiredAlignMask;
	size_t padding = alignedPos - m_pos;
	if(padding > 0)
		Consume(padding);
}

Color MemReader::GetColor(bool swapEndian) 
{
	Color result;
	result.r = Get<float>(swapEndian);
	result.g = Get<float>(swapEndian);
	result.b = Get<float>(swapEndian);
	result.a = Get<float>(swapEndian);
	return result;
}

ColorRGBA MemReader::GetColorRGBA()
{
	ColorRGBA result;
	result.r = Get<uint8_t>(false);
	result.g = Get<uint8_t>(false);
	result.b = Get<uint8_t>(false);
	result.a = Get<uint8_t>(false);
	return result;
}
