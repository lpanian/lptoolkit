#pragma once
#ifndef INCLUDED_toolkit_serialie_HH
#define INCLUDED_toolkit_serialie_HH

#include <vector>
#include <type_traits>
#include <memory>
#include <unordered_map>
#include <list>
#include <cstdint>
#include <cstring>
#include "toolkit/mathcommon.hh"

namespace lptk
{

class Color;
class ColorRGBA;

////////////////////////////////////////////////////////////////////////////////
// padded pointers for compatible structs on 64bit and 32bit machines
template<class T, size_t PtrSize>
struct SerializePtrBase { static_assert(PtrSize == 0, "this class should not be directly instantiated"); };

template<class T>
struct SerializePtrBase<T, 4> { 
	union {
		T* m_ptr;
		uint64_t m_id;
	} m_data ;
};

template<class T>
struct SerializePtrBase<T, 8> {
	union {
		T* m_ptr;
		uint64_t m_id;
	} m_data;
};

template<class T>
struct SerializePtr : public SerializePtrBase<T, sizeof(T*)> { 
	SerializePtr() {}
	SerializePtr(T* ptr) { SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr = ptr; }
	operator T*() { return SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr; }
	operator const T*() const { return SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr; }
	T& operator*() { return *SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr; }
	const T& operator*() const { return *SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr; }
	T* operator->() { return SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr; }
	const T* operator->() const { return SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr; }
	operator bool() const { return SerializePtrBase<T, sizeof(T*)>::m_data.m_ptr != nullptr; }
};

static_assert(sizeof(SerializePtr<char>) == 8, "only pointers of 8 bytes, or 4 bytes + 4 bytes padding are supported for serialization");

////////////////////////////////////////////////////////////////////////////////
struct SerializeFileHeader
{
	char m_tag[4];		// 4 bytes
	uint16_t m_align;		// 2 bytes
	uint16_t m_version;		// 2 bytes
	uint32_t m_ptrTableOffset;	// 4 bytes
	uint32_t m_ptrTableSize;	// 4 bytes
	uint32_t m_dataOffset;	// 4 bytes
	uint32_t m_dataSize;		// 4 bytes
	char m_padding[8];	// 12 bytes 
};
static_assert(sizeof(SerializeFileHeader) == 32, "expected SerializeFileHeader size is 32 bytes");

////////////////////////////////////////////////////////////////////////////////
// MemSerializer - serialize classes into a continguous block of memory
class MemSerializer
{
public:
	MemSerializer(size_t bufferSize, size_t align = 16);
	MemSerializer(MemSerializer&& other);

	void Put(const void* bytes, size_t size);
	void Advance(size_t size);
	void AlignedAdvance(size_t align);

	template<class T>
	void PutVec3(const vec3<T>&);
	void PutColor(const Color&);
	void PutColorRGBA(const ColorRGBA&);

	template<class T>
	void Put(const T& t) {
		ASSERT(std::is_arithmetic<T>::value || std::is_enum<T>::value);
		AlignedAdvance(alignof(T));
		Put(&t, sizeof(t));
	}

	// Returns null if the pointer value has already been written, otherwise returns
	// a serializer to append to.
	MemSerializer* PutPointer(const void* ptr, size_t bufferSize, size_t align = 16);
	template<class T>
	MemSerializer* PutPointer(SerializePtr<const T> ptr, size_t bufferSize, size_t align = alignof(T))
	{
		return PutPointer(ptr.m_data.m_ptr, bufferSize, align);
	}

	size_t GetAlign() const { return m_align; }
	size_t GetPos() const { return m_pos; }
	bool Full() const { return m_mem.size() == m_pos; }
	bool Error() const { return m_error; }
	operator bool() const { return !Full() && !Error(); }

	size_t CalcSize() const;
	void MergeChildren();
	std::vector<char> CreateFileData() ;
private:
	struct PointerData 
	{
		PointerData() : m_dataOffset(-1), m_serializer() {}
		PointerData(PointerData&& other)
			: m_dataOffset(-1)
			, m_serializer(std::move(other.m_serializer))
		{
			std::swap(m_dataOffset, other.m_dataOffset);
		}
		PointerData& operator=(PointerData&& other) {
			m_dataOffset = other.m_dataOffset;
			other.m_dataOffset = -1;
			m_serializer = std::move(other.m_serializer);
			return *this;
		}

		PointerData(std::unique_ptr<MemSerializer>&& serializer) 
			: m_dataOffset(-1)
			, m_serializer(std::move(serializer))
		{}

		ptrdiff_t m_dataOffset;							// offset to data in this m_mem block
		std::unique_ptr<MemSerializer> m_serializer;	// pointer to serializer.
	private:	
		PointerData(const PointerData&) DELETED ;
		PointerData& operator=(const PointerData&) DELETED ;
	};

	size_t m_align;	
	std::vector<char> m_mem;							// serialized memory contained in this object. 
	size_t m_pos;
	bool m_error;
	std::vector<ptrdiff_t> m_pointerOffsets;			// offsets in this block that contain pointers
	std::vector<PointerData> m_pointerData;				// location of pointed-to values by id
	std::unordered_map<const void*, uint64_t> m_pointerIndexMap;	// index into pointerData of written pointers:
														// used to support objects that point to the same
														// object more than once
};

////////////////////////////////////////////////////////////////////////////////
// helper to call serialize functions in classes.
template<class T>
inline void SerializeClass(MemSerializer& serializer, const T& t)
{
	serializer.AlignedAdvance(alignof(T));
	t.SerializeTo(serializer);
}

template<class T>
inline MemSerializer SerializeClass(const T& t)
{
	MemSerializer serializer(sizeof(T), alignof(T));
	SerializeClass(serializer, t);
	return serializer;
}

template<class T, bool IsPOD> struct SerializeClassOrPODHelper {
	static inline void serialize(MemSerializer& serializer, const T& t) 
		{ unused_arg(serializer); SerializeClass(t); }
};
template<class T> struct SerializeClassOrPODHelper<T, true> {
	static inline void serialize(MemSerializer& serializer, const T& t) 
		{ unused_arg(serializer); serializer.Put<T>(t); }
};	

template<class T>
inline void SerializeClassOrPOD(MemSerializer& serializer, const T& t)
{
	SerializeClassOrPODHelper<T,
		std::is_arithmetic<T>::value || std::is_enum<T>::value>::serialize(serializer, t);
}

template<class T>
inline void SerializeClassPtr(MemSerializer& serializer, SerializePtr<const T> t)
{
	const T* ptr = t.m_data.m_ptr;
	MemSerializer* objSerializer = serializer.PutPointer(ptr, sizeof(T), alignof(T));
	if(objSerializer) {
		ptr->SerializeTo(*objSerializer);
	}
}

template<class T>
inline void SerializeClassArray(MemSerializer& serializer, SerializePtr<const T> t, size_t count)
{
	const T* ptr = t.m_data.m_ptr;
	const size_t stride = reinterpret_cast<const char*>(&ptr[1]) - 
		reinterpret_cast<const char*>(&ptr[0]);
	const size_t totalSize = count > 0 ? sizeof(T) + (count-1) * stride : 0;

	MemSerializer* objSerializer = serializer.PutPointer(ptr, totalSize, alignof(T));
	if(objSerializer)
	{
		for(size_t i = 0; i < count; ++i)
			SerializeClass(*objSerializer, ptr[i]);
	}
}

template<class T>
inline void SerializeByteArray(MemSerializer& serializer, SerializePtr<const T> t, size_t numBytes)
{
	const T* ptr = t.m_data.m_ptr;
	MemSerializer *dataSerializer = serializer.PutPointer(ptr, numBytes, alignof(T));
	if(dataSerializer)
		dataSerializer->Put(ptr, numBytes);
}

inline void SerializeStringPtr(MemSerializer& serializer, SerializePtr<const char> str)
{
	const char* ptr = str.m_data.m_ptr;
	size_t sizeBytes = ptr ? strlen(ptr) + 1 : 0;
	MemSerializer* objSerializer = serializer.PutPointer(ptr, sizeBytes, 1);
	if(objSerializer)
	{
		objSerializer->Put(ptr, sizeBytes);
	}
}

////////////////////////////////////////////////////////////////////////////////
// use this to fix memory addresses of a SERL file in memory
class MemPatcher
{
public:
	MemPatcher(const SerializeFileHeader& header,
		char* ptrBuffer, char* dataBuffer);
	operator bool () const { return !m_error; }
	SerializeFileHeader m_header;
private:
	std::vector<char*> m_pointerData;
	bool m_error;
};

////////////////////////////////////////////////////////////////////////////////
class SerializedClassMemory
{
public:
	SerializedClassMemory();
	SerializedClassMemory(SerializedClassMemory&& other);
	SerializedClassMemory& operator=(SerializedClassMemory&&);
	SerializedClassMemory(const SerializedClassMemory&) DELETED;
	SerializedClassMemory& operator=(const SerializedClassMemory&) DELETED;
	operator bool() const { return m_dataMem.get() != nullptr; }

	bool LoadFile(const char* serializedFile, MemPoolId poolid);

	template<class T> 
	T* Reinterpret() {
		return reinterpret_cast<T*>(m_dataMem.get());
	}
private:
	std::unique_ptr<char[]> m_dataMem;	
};

////////////////////////////////////////////////////////////////////////////////
// classes for building 'flat' looking structures by allocating a list of memory chunks
// and then using the memory serializer described above to actually flatten the structure.
struct MemoryChunk 
{
	virtual ~MemoryChunk() {}
};

template<class T>
struct MemoryChunkInst : public MemoryChunk 
{
	// VS has moves, so rely on a move constructor.
	MemoryChunkInst() : m_data() {}
	MemoryChunkInst(T&& existingData)
		: m_data(std::move(existingData))
	{ }
private:
	MemoryChunkInst(const MemoryChunkInst&) DELETED;
	MemoryChunkInst& operator=(const MemoryChunkInst&) DELETED;
public:	
	T m_data;
};

template<class T>
static T* AllocMemoryChunk(std::list<std::unique_ptr<MemoryChunk>>& chunks)
{
	std::unique_ptr<MemoryChunkInst<T>> ptr (new MemoryChunkInst<T>());
	T* result = &ptr->m_data;
	chunks.push_back(std::move(ptr));
	return result;
}
template<class T>
static T* AllocMemoryChunk(std::list<std::unique_ptr<MemoryChunk>>& chunks, T&& existing)
{
	std::unique_ptr<MemoryChunkInst<T>> ptr (new MemoryChunkInst<T>(std::move(existing)));
	T* result = &ptr->m_data;
	chunks.push_back(std::move(ptr));
	return result;
}

template<class T>
void MemSerializer::PutVec3(const vec3<T>& v)
{
	Put<float>(v.x);
	Put<float>(v.y);
	Put<float>(v.z);
}

}

#endif

