#include <cstring>
#include <type_traits>
#include "toolkit/common.hh"
#include "toolkit/memstream.hh"
#include "toolkit/endian.hh"

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
MemWriter::MemWriter() 
{}

MemWriter::MemWriter(char* buffer, size_t size, int flags) 
	: m_b(buffer),m_s(size),m_pos(0),m_error(false),
	 m_flags(flags)
{}
	
char* MemWriter::Put(const void* bytes, size_t size, bool swapEndian) 
{
	using namespace MemFormatFlag;
	if(m_s - m_pos >= size) 
    {
        auto resultVal = &m_b[m_pos];
		memcpy(resultVal,bytes,size);
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
        return resultVal;
	}
    else
    {
        m_error = true;
        return nullptr;
    }
}

char* MemWriter::Advance(size_t size) 
{
	if(m_s - m_pos >= size) 
    {
        auto resultVal = &m_b[m_pos];
		m_pos += size;
        return resultVal;
	} 
    else
    {
        m_error = true;
        return nullptr;
    }
}

char* MemWriter::AlignedAdvance(size_t align)
{
	const size_t requiredAlignMask = align - 1;
	const size_t alignedPos = (m_pos + requiredAlignMask) & ~requiredAlignMask;
	const size_t padding = alignedPos - m_pos;
    if (padding > 0)
        return Advance(padding);
    else
        return (!Full() && !Error()) ? &m_b[m_pos] : nullptr;
}
	
char* MemWriter::PutColor(const Color& c, bool swapEndian)
{
	auto start = Put<float>(c.r, swapEndian);
	Put<float>(c.g, swapEndian);
	Put<float>(c.b, swapEndian);
	Put<float>(c.a, swapEndian);
    return start;
}

char* MemWriter::PutColorRGBA(ColorRGBA c)
{
	auto start = Put<uint8_t>(c.r, false);
	Put<uint8_t>(c.g, false);
	Put<uint8_t>(c.b, false);
	Put<uint8_t>(c.a, false);
    return start;
}



////////////////////////////////////////////////////////////////////////////////
MemReader::MemReader() 
{}

MemReader::MemReader(const char* buffer, size_t size, int flags) 
	: m_top(buffer)
    , m_topSize(size)
    , m_b(buffer)
    , m_s(size)
    , m_error(false)
    , m_flags(flags) 
{}
    
MemReader::MemReader(const char* top, size_t topSize, const char* buffer, size_t size, int flags)
    : m_top(top)
    , m_topSize(topSize)
    , m_b(buffer)
    , m_s(size)
    , m_error(false)
    , m_flags(flags)
{
}

void MemReader::Get(void* bytes, size_t size, bool swapEndian) 
{
	using namespace MemFormatFlag;
	if( (m_s - m_pos) >= size) 
	{
		memcpy(bytes,&m_b[m_pos],size);
		if(swapEndian)
		{
			if(IsNativeLittleEndian() && (m_flags & FLAG_BigEndianData) != 0)
				SwapEndian(bytes, size);
			else if(IsNativeBigEndian() && (m_flags & FLAG_LittleEndianData) != 0)
				SwapEndian(bytes, size);
		}
		m_pos += size;
	}
    else
    {
        memset(bytes, 0, size);
        m_error = true;
    }
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
    
MemReader MemReader::GetSubReader(int flags) const
{
    if (m_error)
        return MemReader();
    else
        return MemReader(m_top, m_topSize, m_b + m_pos, m_s - m_pos, flags ? flags : m_flags);
}

MemReader MemReader::GetSubReader(size_t offset, size_t requestedSize, int flags) const
{
    if (m_error)
        return MemReader();
    else if (m_pos + offset + requestedSize <= m_topSize)
        return MemReader(m_top, m_topSize, m_b + m_pos + offset, requestedSize, flags ? flags : m_flags);
    else
        return MemReader();
}

MemReader MemReader::GetTopReader(size_t offset, size_t requestedSize, int flags) const
{
    if (m_error)
        return MemReader();
    else if (offset + requestedSize <= m_topSize)
        return MemReader(m_top, m_topSize, m_top + offset, requestedSize, flags ? flags : m_flags);
    else
        return MemReader();
}
    

}

