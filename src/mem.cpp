#include "toolkit/common.hh"
#include "toolkit/mem.hh"
#include <atomic>
#include <limits>
#include "toolkit/mathcommon.hh"
//#include <malloc.h>

//#define PLAIN_MALLOC

static const char *g_poolName[] = {
	"General",
	"Temp",
	"String",
	"Debug",
};
static_assert(MEMPOOL_NUM == ARRAY_SIZE(g_poolName), "size mismatch for pool names");

static std::atomic<std::size_t> g_poolMemUsed[MEMPOOL_NUM];

////////////////////////////////////////////////////////////////////////////////
void* mem_allocate(std::size_t n, MemPoolId id, unsigned int align)
{
	ASSERT(align > 0 && IsPower2(align));
#ifdef PLAIN_MALLOC
	return malloc(n);

	std::size_t totalSize = n + align;
	void* p = malloc(totalSize);
	if(!p) return nullptr;
	std::size_t ptrVal = reinterpret_cast<size_t>(p);
	std::size_t aligned = (ptrVal + align) & ~std::size_t(align-1);
	std::size_t padding = aligned - ptrVal;
	std::size_t locPadding = aligned-1;
	*reinterpret_cast<uint8_t*>(locPadding) = uint8_t(padding);
	return reinterpret_cast<void*>(aligned);
#else
	ASSERT(id < MEMPOOL_NUM);

	// header:
	// allocSize: uint32_t
	// allocPool: uint8_t
	// ... padding ...
	// paddingOffset: uint8_t
	// ... data ... <-- result
	constexpr std::size_t headerSize = sizeof(uint32_t) + sizeof(uint8_t);
	std::size_t totalSize = n + headerSize + align;
	if(totalSize > std::numeric_limits<uint32_t>::max()) 
	{
		ASSERT(false);
		return nullptr;
	}
	void* p = malloc(totalSize);
	if(!p) return nullptr;

	const std::size_t ptrHeader = reinterpret_cast<std::size_t>(p);
	const std::size_t alignMask = ~std::size_t(align - 1);
	const std::size_t ptrData = (ptrHeader + headerSize + align) & alignMask;
	const std::size_t padding = ptrData - ptrHeader;
	const std::size_t locSize = ptrHeader;
	const std::size_t locPool = ptrHeader + sizeof(uint32_t);
	const std::size_t locPadding = ptrData - 1;
	ASSERT(padding <= std::numeric_limits<uint8_t>::max());
	ASSERT(padding >= headerSize);
	ASSERT(locPool < locPadding);
	ASSERT(totalSize - padding >= n);
	*reinterpret_cast<uint32_t*>(locSize) = totalSize;
	*reinterpret_cast<uint8_t*>(locPool) = uint8_t(id);
	*reinterpret_cast<uint8_t*>(locPadding) = uint8_t(padding);

	g_poolMemUsed[id].fetch_add(totalSize);

//	std::cout << "allocated: " << std::hex << ptrHeader << " (" << ptrData << ")" << 
//		" in pool " << g_poolName[id] << 
//		" of size " << std::dec << totalSize << " with padding " << padding << std::endl;
	return reinterpret_cast<void*>(ptrData);
#endif
}

void mem_free(void* p)
{
	if(!p) return;
#ifdef PLAIN_MALLOC
	free(p); return;
	const std::size_t ptrData = reinterpret_cast<std::size_t>(p);
	const std::size_t locPadding = ptrData-1;
	uint8_t padding = *reinterpret_cast<uint8_t*>(locPadding);
	const std::size_t original = ptrData - padding;
	free(reinterpret_cast<void*>(original));
#else
	const std::size_t ptrData = reinterpret_cast<std::size_t>(p);
	const std::size_t padding = *reinterpret_cast<uint8_t*>(ptrData-1);
	const std::size_t ptrHeader = ptrData - padding;
	const std::size_t locPool = ptrHeader + sizeof(uint32_t);
	const std::size_t allocedSize = *reinterpret_cast<uint32_t*>(ptrHeader);
	const uint8_t poolId = *reinterpret_cast<uint8_t*>(locPool);

	ASSERT(poolId < MEMPOOL_NUM);

	g_poolMemUsed[poolId].fetch_sub(allocedSize);
//	std::cout << "freed: " << std::hex << ptrHeader << std::endl;
	free(reinterpret_cast<void*>(ptrHeader));
#endif
}

////////////////////////////////////////////////////////////////////////////////
void *operator new(std::size_t n) 
{
	return mem_allocate(n, MEMPOOL_General, 16);
}

void *operator new[](std::size_t n)
{
	return mem_allocate(n, MEMPOOL_General, 16);
}

void *operator new(std::size_t n, MemPoolId id, unsigned int align) 
{
	return mem_allocate(n, id, align);
}

void *operator new[](std::size_t n, MemPoolId id, unsigned int align) 
{
	return mem_allocate(n, id, align);
}

void operator delete(void* p) 
{
	mem_free(p);
}

void operator delete[](void* p) 
{
	mem_free(p);
}


size_t mem_GetSizeAllocated(MemPoolId id)
{
	ASSERT(id < MEMPOOL_NUM);
	return g_poolMemUsed[id];
}

void mem_ReportText(const char* title)
{
	std::cout << std::dec;
	std::cout << "Tagged memory usage " << (title ? title : "")  << ":" << std::endl;
	for(int i = 0; i < MEMPOOL_NUM; ++i)
	{
		std::cout << g_poolName[i] << " : " << g_poolMemUsed[i] << std::endl;
	}
}

