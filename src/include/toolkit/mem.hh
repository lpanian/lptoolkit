#pragma once

#include <new>

namespace lptk 
{
    enum MemPoolId 
    {
        MEMPOOL_General,			// default allocations go here.
        MEMPOOL_Temp,				// memory that shouldn't be left sitting around
        MEMPOOL_String,				// default memory used by Str class
        MEMPOOL_Debug,				// debugging memory
        MEMPOOL_Network,                        // related to network
        MEMPOOL_NUM,
    };
}

// Pool operators
void *operator new(size_t n, lptk::MemPoolId id, unsigned int align) ;
void *operator new[](size_t n, lptk::MemPoolId id, unsigned int align) ;

namespace lptk 
{
    // default placement new and delete are defined in <new>
    size_t mem_GetSizeAllocated(MemPoolId id);
    void mem_ReportText(const char* title = nullptr);

    // basic allocate functions
    void* mem_allocate(size_t n, MemPoolId id, unsigned int align);
    void* mem_reallocate(void* previous, size_t n, MemPoolId id, unsigned int align);
    void mem_free(void* p);

    // raw allocate functions - wrapper around whatever we use to allocate (usually malloc/free)
    void* raw_allocate(size_t n);
    void raw_free(void* p);
}

