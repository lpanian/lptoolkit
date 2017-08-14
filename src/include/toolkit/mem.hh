#pragma once
#ifndef INCLUDED_toolkit_mem_hh
#define INCLUDED_toolkit_mem_hh

#include <cstddef>
#include <memory>
#include <limits>

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

// Default operators
void *operator new(size_t n) ;
void *operator new[](size_t n) ;
void *operator new(size_t n, const std::nothrow_t&) ;
void *operator new[](size_t n, const std::nothrow_t&) ;

// Pool operators
void *operator new(size_t n, lptk::MemPoolId id, unsigned int align) ;
void *operator new[](size_t n, lptk::MemPoolId id, unsigned int align) ;

// Deletes
void operator delete(void* p) noexcept ;
void operator delete[](void* p) noexcept ;

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


    template< typename T, MemPoolId PoolId, unsigned int Align = 1> 
    class MemPoolSTLAlloc
    {
    public:
        typedef T value_type;
        typedef T* pointer;
        typedef const T* const_pointer;
        typedef T& reference;
        typedef const T& const_reference;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        pointer address(reference r) const { return &r; }
        const_pointer address(const_reference r) const { return &r; }
        pointer allocate(size_type n, std::allocator<void>::const_pointer hint = nullptr) {
            (void)hint;
            return reinterpret_cast<pointer>(mem_allocate(n * sizeof(T), PoolId, Align));
        }
        void deallocate(pointer p, size_type n) {
            (void)n;
            mem_free(p);
        }
        size_type max_size() const { return std::numeric_limits<size_type>::max(); }

        template< class U, class... Args >
            void construct(U* p, Args&&... args) { new ((void*)p) U(std::forward<Args>(args)...); }

        void destroy(pointer p) { (void)p; reinterpret_cast<T*>(p)->~T(); }

        template<class U>
            struct rebind { typedef MemPoolSTLAlloc<U, PoolId, Align> other; };

        bool operator==(const MemPoolSTLAlloc&) const { return true; }
        bool operator!=(const MemPoolSTLAlloc&) const { return false; }
    };

}

#endif

