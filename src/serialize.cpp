#include <algorithm>
#include <cstdio>
#include <iostream>
#include "toolkit/memstream.hh"
#include "toolkit/serialize.hh"

static constexpr int kSerializeFileVersion = 1;

namespace lptk
{

// format:
// header (32 bytes) 
// ptrTable: 
//  4 bytes numEntries 					// table that maps ids to offsets in data
//  4 * numEntries bytes: ptrDataOffset
//  4 bytes numEntries 					// table with locations of SerializePtrs in data
//  4 * numEntries bytes: ptrOffset
// data: just bytes, yo! not aligned in the file, but should be copied to an aligned place in memory

// On read, everything after the header is stuffed into memory that is aligned as specified.
// Then, the first ptr table is parsed to build a map from ids to addresses
// the second ptr table is parsed to fixup each address.

MemSerializer::MemSerializer(size_t bufferSize, size_t align)
	: m_align(align)
	, m_mem(bufferSize)
	, m_pos(0)
	, m_error(false)
	, m_pointerOffsets()
	, m_pointerData()
	, m_pointerIndexMap()
{
}
	
MemSerializer::MemSerializer(MemSerializer&& other)
	: m_align(0)
	, m_mem()
	, m_pos(0)
	, m_error(false)
	, m_pointerOffsets()
	, m_pointerData()
	, m_pointerIndexMap()
{
	std::swap(m_align, other.m_align);
	m_mem.swap(other.m_mem);
	std::swap(m_pos, other.m_pos);
	std::swap(m_error, other.m_error);
	m_pointerOffsets.swap(other.m_pointerOffsets);
	m_pointerData.swap(other.m_pointerData);
	m_pointerIndexMap.swap(other.m_pointerIndexMap);
}

void MemSerializer::Put(const void* bytes, size_t size)
{
	const size_t bufSize = m_mem.size();
	if(bufSize - m_pos >= size) {
		memcpy(&m_mem[m_pos], bytes, size);
		m_pos += size;
	} else 
		m_error = true;
}

void MemSerializer::Advance(size_t size)
{
	const size_t bufSize = m_mem.size();
	if(bufSize - m_pos >= size) 
		m_pos += size;
	else
		m_error = true;
}

void MemSerializer::AlignedAdvance(size_t align)
{
	const size_t requiredAlignMask = align - 1;
	const size_t alignedPos = (m_pos + requiredAlignMask) & ~requiredAlignMask;
	const size_t padding = alignedPos - m_pos;
	if(padding > 0)
		Advance(padding);
}

void MemSerializer::PutColor(const Color& c)
{
	Put<float>(c.r);
	Put<float>(c.g);
	Put<float>(c.b);
	Put<float>(c.a);
}

void MemSerializer::PutColorRGBA(const ColorRGBA& c)
{
	Put<uint8_t>(c.r);
	Put<uint8_t>(c.g);
	Put<uint8_t>(c.b);
	Put<uint8_t>(c.a);
}

MemSerializer* MemSerializer::PutPointer(const void* ptr, size_t bufferSize, size_t align)
{
	// non-null ptr
	AlignedAdvance(alignof(uint64_t));
	if(ptr)
	{
		m_pointerOffsets.push_back(m_pos);

		uint64_t id = 0;
		auto it = m_pointerIndexMap.find(ptr);
		if(it != m_pointerIndexMap.end()) 
		{
			id = it->second;
			Put<uint64_t>(id);
			return nullptr;
		}
		else {
			id = m_pointerData.size() + 1;
			Put<uint64_t>(id);
			m_pointerIndexMap[ptr] = id;
			m_pointerData.emplace_back(std::unique_ptr<MemSerializer>(new MemSerializer(bufferSize, align)));
			return m_pointerData.back().m_serializer.get();
		}
	}
	else // just write 0 and skip the fixup by not putting in the pointer list
	{
		Put<uint64_t>(0);
		return nullptr;
	}
}

size_t MemSerializer::CalcSize() const
{
	size_t requiredSize = m_mem.size();
	for(const auto& ptrData: m_pointerData)
	{
		if(ptrData.m_serializer)
		{
			requiredSize = AlignValue(requiredSize, ptrData.m_serializer->GetAlign());
			requiredSize += ptrData.m_serializer->CalcSize();
		}
	}
	return requiredSize;
}
	
void MemSerializer::MergeChildren()
{
	// ensure that all children are merged.
	for(auto& ptrData: m_pointerData)
	{
		if(ptrData.m_serializer)
			ptrData.m_serializer->MergeChildren();
	}

	const auto oldSize = m_mem.size();
	const auto requiredSize = CalcSize();
	ASSERT(requiredSize >= m_mem.size());
	m_mem.resize(requiredSize);

	// merge child memory into this serializer's data
	auto curOffset = oldSize;
	for(auto i = size_t(0), c = m_pointerData.size(); i < c; ++i)
	{
		auto* ptrData = &m_pointerData[i];
		if(ptrData->m_serializer)
		{
			// copy byte data
			const size_t startOfBlock = AlignValue(curOffset, ptrData->m_serializer->GetAlign());
			memcpy(&m_mem[startOfBlock], &ptrData->m_serializer->m_mem[0], ptrData->m_serializer->m_mem.size());

			const size_t numOffsets = m_pointerOffsets.size();
			const size_t numPtrData = m_pointerData.size();

			// copy pointer locations and offset them to the new block
			m_pointerOffsets.insert(m_pointerOffsets.end(), 
				ptrData->m_serializer->m_pointerOffsets.begin(), ptrData->m_serializer->m_pointerOffsets.end());
			auto firstNewOffset = m_pointerOffsets.begin() + numOffsets;
			std::for_each(firstNewOffset, m_pointerOffsets.end(), [&](ptrdiff_t& old){ old += startOfBlock; });

			// iterate over the pointer offsets and update their ids so they match with the new ids.
			std::for_each(firstNewOffset, m_pointerOffsets.end(), [&](const ptrdiff_t& ptrOffset) {
				ASSERT(ptrOffset < (int)m_mem.size());
				uint64_t* idPtr = reinterpret_cast<uint64_t*>(&m_mem[ptrOffset]);
				ASSERT(*idPtr <= ptrData->m_serializer->m_pointerData.size());
				if(*idPtr) 
					*idPtr = *idPtr + static_cast<uint64_t>(numPtrData);
			});

			// copy children info, and offset each data offset appropriately
			ASSERT(ptrData->m_serializer.get() != nullptr);
			m_pointerData.resize(m_pointerData.size() + ptrData->m_serializer->m_pointerData.size());
			
			ptrData = &m_pointerData[i];
			auto firstNewPtrData = m_pointerData.begin() + numPtrData;
			std::move(ptrData->m_serializer->m_pointerData.begin(), 
				ptrData->m_serializer->m_pointerData.end(), 
				firstNewPtrData);
			std::for_each(firstNewPtrData, m_pointerData.end(), [&](PointerData& old) { 
				ASSERT(!old.m_serializer); 
				old.m_dataOffset += startOfBlock;
			});

			// free the serializer now that we've merged.
			curOffset = startOfBlock + ptrData->m_serializer->m_mem.size();
			ptrData->m_dataOffset = startOfBlock;
			ptrData->m_serializer.reset();
		}
	}
}

std::vector<char> MemSerializer::CreateFileData() 
{
	// return an empty data set and set error flag if this serializer hasn't been merged.
	for(const auto& ptrData: m_pointerData)
	{
		if(ptrData.m_serializer)
		{
			m_error = true;
			return std::vector<char>();
		}
	}

	const auto ptrTableSize = 
        static_cast<uint32_t>(
        2 * sizeof(uint32_t)	+
		sizeof(uint32_t) * m_pointerOffsets.size() +
		sizeof(uint32_t) * m_pointerData.size());
		
	const size_t requiredSize = 
		sizeof(SerializeFileHeader) + 
		ptrTableSize +
		m_mem.size() ;
	std::vector<char> fileData(requiredSize);


	const auto ptrTableOffset = static_cast<uint32_t>(sizeof(SerializeFileHeader));
	const auto dataOffset = ptrTableOffset + ptrTableSize;
	
	MemWriter writer(&fileData[0], fileData.size());
	writer.Put("SERL", 4, false);			// m_tag
	writer.Put<uint16_t>(uint16_t(m_align));				// m_align
	writer.Put<uint16_t>(kSerializeFileVersion);	// m_version
	writer.Put<uint32_t>(ptrTableOffset);		// m_ptrTableOffset
	writer.Put<uint32_t>(ptrTableSize);			// m_ptrTableSize
	writer.Put<uint32_t>(dataOffset);			// m_dataOffset
	writer.Put<uint32_t>(static_cast<uint32_t>(m_mem.size()));			// m_dataSize
	writer.Advance(8);						// m_padding

	// ptr table
	ASSERT(ptrTableOffset == writer.GetPos());

	// ptr map. Array of offsets that correspond to objects that are pointed to.
	// ids are index+1, and the value is the offset that contains the object.
	writer.Put<uint32_t>(static_cast<uint32_t>(m_pointerData.size()));
	for(auto i = size_t(0), c = m_pointerData.size(); i < c; ++i)
		writer.Put<uint32_t>(static_cast<uint32_t>(m_pointerData[i].m_dataOffset));
	
	// ptr locations (offsets in data that contain a pointer)
	writer.Put<uint32_t>(static_cast<uint32_t>(m_pointerOffsets.size()));
	for(auto i = size_t(0), c = m_pointerOffsets.size(); i < c; ++i)
		writer.Put<uint32_t>(static_cast<uint32_t>(m_pointerOffsets[i]));

	// data offset
	ASSERT(dataOffset == writer.GetPos());
	writer.Put(&m_mem[0], m_mem.size(), false);

	return fileData;
}

////////////////////////////////////////////////////////////////////////////////
MemPatcher::MemPatcher(const SerializeFileHeader& header,
	char* ptrBuffer, char* dataBuffer)
	: m_pointerData()
	, m_error(false)
{
	MemReader ptrReader(ptrBuffer, header.m_ptrTableSize);
	// ptr map
	uint32_t numAddressMaps = ptrReader.Get<uint32_t>();
	m_pointerData.resize(numAddressMaps, nullptr);
	for(uint32_t i = 0; i < numAddressMaps; ++i)
	{
		uint32_t offset = ptrReader.Get<uint32_t>();
		ASSERT(offset < header.m_dataSize);
		m_pointerData[i] = dataBuffer + offset;
	}

	// ptr loc & fixup
	uint32_t numPtrLocs = ptrReader.Get<uint32_t>();
	for(uint32_t i = 0; i < numPtrLocs; ++i)
	{
		uint32_t offset = ptrReader.Get<uint32_t>();
		ASSERT(offset < header.m_dataSize);

		// first read the stored Id
		uint64_t* ptrId = reinterpret_cast<uint64_t*>(dataBuffer + offset);
		// id == 0 means a nullptr
		if(*ptrId > 0)
		{
			uint32_t ptrIndex = uint32_t(*ptrId) - 1;
			ASSERT(ptrIndex < numAddressMaps);

			// then write the address back to that location
			char** ptrVal = reinterpret_cast<char**>(ptrId);
			*ptrVal = m_pointerData[ptrIndex];
		}
	}

	if(ptrReader.Error())
		m_error = true;
}

////////////////////////////////////////////////////////////////////////////////
SerializedClassMemory::SerializedClassMemory()
	: m_dataMem()
{
}

SerializedClassMemory::SerializedClassMemory(SerializedClassMemory&& other)
	: m_dataMem()
{
	m_dataMem = std::move(other.m_dataMem);
}
	
SerializedClassMemory& SerializedClassMemory::operator=(SerializedClassMemory&& other)
{
	m_dataMem = std::move(other.m_dataMem);
	return *this;
}

bool SerializedClassMemory::LoadFile(const char* serializedFile, MemPoolId poolId)
{
	FILE* fp = nullptr;
#ifdef USING_VS
	if(0 != fopen_s(&fp, serializedFile, "rb"))
		return false;
#else
	fp = fopen(serializedFile, "rb");
#endif
	if(!fp) return false;
	auto closeOnExit = at_scope_exit([&]{ fclose(fp); });

	SerializeFileHeader header;
	const size_t numReadHeader = fread(&header, 1, sizeof(header), fp);
	if(numReadHeader != sizeof(header))
		return false;

	if(memcmp(header.m_tag, "SERL", 4) != 0)
		return false;

	if(header.m_version != kSerializeFileVersion)
	{
		std::cerr << "SERL verison mismatch: expected " << kSerializeFileVersion << ", found " << header.m_version
			<< std::endl;
		return false;
	}

	std::unique_ptr<char[]> ptrTableBuffer (new (MEMPOOL_Temp, 4) char[header.m_ptrTableSize]);
	fseek(fp, header.m_ptrTableOffset, SEEK_SET);
	const size_t numReadPtrTable = fread(&ptrTableBuffer[0], 1, header.m_ptrTableSize, fp);
	if(numReadPtrTable != header.m_ptrTableSize)
		return false;
	
	std::unique_ptr<char[]> mainData (new (poolId, header.m_align) char[header.m_dataSize]);
	if(!mainData) 
		return false;

	fseek(fp, header.m_dataOffset, SEEK_SET);
	const size_t numReadData = fread(mainData.get(), 1, header.m_dataSize, fp);
	if(numReadData != header.m_dataSize)
		return false;

	MemPatcher patcher(header, ptrTableBuffer.get(), mainData.get());
	if(!patcher)
		return false;

	m_dataMem = std::move(mainData);
	return true;
}

}

