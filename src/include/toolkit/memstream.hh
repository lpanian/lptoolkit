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

	void Put(const void* bytes, size_t size, bool swapEndian = true);
	void Advance(size_t size);
	void AlignedAdvance(size_t align);

	template<class T>
	void Put(const T& t, bool swapEndian = true) {
		ASSERT(std::is_arithmetic<T>::value || std::is_enum<T>::value);
		AlignedAdvance(alignof(T));
		Put(&t, sizeof(t), swapEndian);
	}
	template<class T> 
	void PutVec3(const vec3<T>&, bool swapEndian = true);
	void PutColor(const Color&, bool swapEndian = true);
	void PutColorRGBA(ColorRGBA);

	size_t GetPos() const { return m_pos; }

	bool Full() const { return m_s == m_pos; }
	bool Error() const { return m_error; }
};

////////////////////////////////////////////////////////////////////////////////
class MemReader {
	const char* m_b;
	size_t m_s;
	size_t m_pos;
	bool m_error;
	int m_flags;
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
		ASSERT(std::is_arithmetic<T>::value || std::is_enum<T>::value);
		AlignedConsume(alignof(T));
		T result;
		Get(&result, sizeof(T), swapEndian);
		return result;
	}

	size_t GetPos() const { return m_pos; }

	bool Empty() const { return m_s == m_pos; }	   
	bool Error() const { return m_error; }
};

template<class T> 
void MemWriter::PutVec3(const vec3<T>& v, bool swapEndian)
{
	Put<T>(v.x, swapEndian);
	Put<T>(v.y, swapEndian);
	Put<T>(v.z, swapEndian);
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

