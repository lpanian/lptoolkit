#pragma once
#ifndef INCLUDED_toolkit_memstream_HH
#define INCLUDED_toolkit_memstream_HH

#include <type_traits>
#include "vec.hh"
#include "common.hh"
#include "color.hh"

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
namespace MemFormatFlag
{
	enum Type : uint32_t 
	{
		FLAG_BigEndianData = 0x1, // data being read is big-endian, convert to native.
		FLAG_LittleEndianData = 0x2, // data being read is little-endian, convert to native
        FLAG_AlignedConsume = 0x4, // align types when consumingm emory
	};
}

////////////////////////////////////////////////////////////////////////////////
class MemWriter 
{
	char* m_b;
	size_t m_s;
	size_t m_pos;
	bool m_error;
	int m_flags;
public:
	MemWriter(); 
	MemWriter(char* buffer, size_t size, int flags = 0);
	
	explicit operator bool() const { return !Full() && !Error(); }

	char* Put(const void* bytes, size_t size, bool swapEndian = true);
	char* Advance(size_t size);
	char* AlignedAdvance(size_t align);

	template<class T>
	char* Put(const T& t, bool swapEndian = true) {
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "type is not numeric");
		AlignedAdvance(alignof(T));
		return Put(&t, sizeof(t), swapEndian);
	}
	template<class T> 
	char* PutVec3(const vec3<T>&, bool swapEndian = true);
	char* PutColor(const Color&, bool swapEndian = true);
	char* PutColorRGBA(ColorRGBA);

	size_t GetPos() const { return m_pos; }

	bool Full() const { return m_s == m_pos; }
	bool Error() const { return m_error; }
};

////////////////////////////////////////////////////////////////////////////////
class MemReader {
    const char* m_top = nullptr;
    size_t m_topSize = 0;
	const char* m_b = nullptr;
	size_t m_s = 0;
	size_t m_pos = 0;
	bool m_error = true;
	int m_flags = 0;
    MemReader(const char* top, size_t topSize, const char* buffer, size_t size, int flags);
public:
	MemReader();
	MemReader(const char* buffer, size_t size, int flags = 0);
	explicit operator bool() const { return !Empty() && !Error(); }

	void Get(void* bytes, size_t size, bool swapEndian = true);
	void Consume(size_t size);
	void AlignedConsume(size_t align) ; // consume bytes so the next value to bread has the given alignment.

	template<class T>
	vec3<T> GetVec3(bool swapEndian = true) ;
	Color GetColor(bool swapEndian = true) ;
	ColorRGBA GetColorRGBA();

	template<class T>
	T Get(bool swapEndian = true) {
		static_assert(std::is_arithmetic<T>::value || std::is_enum<T>::value, "type is not numeric");
		if(0 != (m_flags & MemFormatFlag::FLAG_AlignedConsume)) 
            AlignedConsume(alignof(T));
		T result;
		Get(&result, sizeof(T), swapEndian);
		return result;
	}

	size_t GetPos() const { return m_pos; }
    const char* GetPtr() const { return m_b + m_pos; }

	bool Empty() const { return m_s == m_pos; }	   
	bool Error() const { return m_error; }

    MemReader GetSubReader(int flags = 0) const;
    MemReader GetSubReader(size_t offset, size_t requestedSize, int flags = 0) const;
    MemReader GetTopReader(size_t offset, size_t requestedSize, int flags = 0) const;
};

template<class T> 
char* MemWriter::PutVec3(const vec3<T>& v, bool swapEndian)
{
	auto start = Put<T>(v.x, swapEndian);
	Put<T>(v.y, swapEndian);
	Put<T>(v.z, swapEndian);
    return start;
}

template<class T>
vec3<T> MemReader::GetVec3(bool swapEndian) 
{
	vec3<T> result;
	result.x = Get<T>(swapEndian);
	result.y = Get<T>(swapEndian);
	result.z = Get<T>(swapEndian);
	return result;
}
	
}

#endif

