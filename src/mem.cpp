#include "toolkit/common.hh"
#include "toolkit/mem.hh"
#include <new>
#include <atomic>
#include <limits>
#include <cstdint>
#include "toolkit/mathcommon.hh"
//#include <malloc.h>

//#define PLAIN_MALLOC

static const char *g_poolName[] = {
	"General",
	"Temp",
	"String",
	"Debug",
    "Network",
};
static_assert(lptk::MEMPOOL_NUM == ARRAY_SIZE(g_poolName), "size mismatch for pool names");

static std::atomic<size_t> g_poolMemUsed[lptk::MEMPOOL_NUM];

namespace lptk 
{
    constexpr size_t kHeaderSize = sizeof(uint32_t) + sizeof(uint8_t);
    // header:
    // allocSize: uint32_t
    // allocPool: uint8_t
    // ... padding ...
    // paddingOffset: uint8_t
    // ... data ... <-- result

    ////////////////////////////////////////////////////////////////////////////////
    inline static void extractHeader(
            const void* p,
        void *& realp,
        size_t& outPadding,
        size_t& outAllocedSize,
        uint8_t& outPoolId)
    {
        const size_t ptrData = reinterpret_cast<size_t>(p);
        const size_t padding = *reinterpret_cast<uint8_t*>(ptrData-1);
        const size_t ptrHeader = ptrData - padding;
        const size_t locPool = ptrHeader + sizeof(uint32_t);
        const size_t allocedSize = *reinterpret_cast<uint32_t*>(ptrHeader);
        const uint8_t poolId = *reinterpret_cast<uint8_t*>(locPool);
        
        realp = reinterpret_cast<void*>(ptrHeader);
        outPadding = padding;
        outAllocedSize = allocedSize;
        outPoolId = poolId;
    }

    inline static size_t computeTotalSize(
            size_t requestSize,
            size_t align)
    {
        size_t totalSize = requestSize + kHeaderSize + align;
        if(totalSize > std::numeric_limits<uint32_t>::max()) 
            ASSERT(false);
        return totalSize;
    }

    inline static void* writeHeader(
       void* p,
       MemPoolId id,
       size_t n,
       size_t totalSize,
       size_t align)
    {
        (void)n;
        const size_t ptrHeader = reinterpret_cast<size_t>(p);
        const size_t alignMask = ~size_t(align - 1);
        const size_t ptrData = (ptrHeader + kHeaderSize + align) & alignMask;
        const size_t padding = ptrData - ptrHeader;
        
        const size_t locSize = ptrHeader;
        const size_t locPool = ptrHeader + sizeof(uint32_t);
        const size_t locPadding = ptrData - 1;
        
        ASSERT(padding <= std::numeric_limits<uint8_t>::max());
        ASSERT(padding >= kHeaderSize);
        ASSERT(locPool < locPadding);
        ASSERT(totalSize - padding >= n);
        
        *reinterpret_cast<uint32_t*>(locSize) = static_cast<uint32_t>(totalSize);
        *reinterpret_cast<uint8_t*>(locPool) = static_cast<uint8_t>(id);
        *reinterpret_cast<uint8_t*>(locPadding) = static_cast<uint8_t>(padding);
        
        return reinterpret_cast<void*>(ptrData);
    }

    ////////////////////////////////////////////////////////////////////////////////
    void* mem_allocate(size_t n, MemPoolId id, unsigned int align)
    {
        ASSERT(align > 0 && IsPower2(align));
#ifdef PLAIN_MALLOC
        return malloc(n);
#else
        ASSERT(id < MEMPOOL_NUM);

        const size_t totalSize = computeTotalSize(n, align);
        void* p = raw_allocate(totalSize);
        if(!p) return nullptr;

        void *result = writeHeader(p, id, n, totalSize, align);
        g_poolMemUsed[id].fetch_add(totalSize);

        //	std::cout << "allocated: " << std::hex << ptrHeader << " (" << ptrData << ")" << 
        //		" in pool " << g_poolName[id] << 
        //		" of size " << std::dec << totalSize << " with padding " << padding << std::endl;
        return result;
#endif
    }

    void* mem_reallocate(void* previous, size_t n, MemPoolId id, unsigned int align)
    {
#ifdef PLAIN_MALLOC
        return realloc(previous, n);
#else
        if(!previous)
            return mem_allocate(n, id, align);

        void* realp = nullptr;
        size_t padding, allocedSize;
        uint8_t poolId;
        extractHeader(previous, realp, padding, allocedSize, poolId);

        ASSERT(realp != nullptr);

        // if realp isn't aligned as we'd like, then make sure we realloc.
        const size_t realpBits = reinterpret_cast<size_t>(realp);
        const size_t alignResult = (realpBits & ~(size_t(align)-1));

        const size_t totalSize = computeTotalSize(n, align);
        void* newp = alignResult == 0 ? realloc(realp, totalSize) : raw_allocate(totalSize);
        
        void* result = nullptr;
        if(newp != realp)
        {
            result = writeHeader(newp, id, n, totalSize, align);
            memcpy(result, previous, allocedSize - padding);
            mem_free(previous);
        }
        else
        {
            g_poolMemUsed[poolId].fetch_sub(allocedSize);
            result = writeHeader(realp, id, n, totalSize, align);
            ASSERT(result == previous);
        }
        
        g_poolMemUsed[id].fetch_add(totalSize);
        return result;
#endif
    }

    void mem_free(void* p)
    {
        if(!p) return;
#ifdef PLAIN_MALLOC
        free(p); 
        return;
#else
        void* realp = nullptr;
        size_t padding, allocedSize;
        uint8_t poolId;
        extractHeader(p, realp, padding, allocedSize, poolId);

        ASSERT(poolId < MEMPOOL_NUM);

        g_poolMemUsed[poolId].fetch_sub(allocedSize);
        raw_free(realp);
#endif
    }
    
    void* raw_allocate(size_t n)
    {
        return malloc(n);
    }

    void raw_free(void* p)
    {
        free(p);
    }

}

////////////////////////////////////////////////////////////////////////////////
void *operator new(size_t n, lptk::MemPoolId id, unsigned int align) 
{
    return lptk::mem_allocate(n, id, align);
}

void *operator new[](size_t n, lptk::MemPoolId id, unsigned int align) 
{
    return lptk::mem_allocate(n, id, align);
}

////////////////////////////////////////////////////////////////////////////////
namespace lptk
{
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

}

