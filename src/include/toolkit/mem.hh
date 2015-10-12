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

    // raw allocate functions
    void* mem_allocate(size_t n, MemPoolId id, unsigned int align);
    void* mem_reallocate(void* previous, size_t n, MemPoolId id, unsigned int align);
    void mem_free(void* p);

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

#ifdef USING_VS
        template< class U >
            void construct(U* p) { new ((void*)p) U; }
        template< class U, class A0 >
            void construct(U* p, A0&& a0) { new ((void*)p) U(std::forward<A0>(a0)); }
        template< class U, class A0, class A1 >
            void construct(U* p, A0&& a0, A1&& a1) { new ((void*)p) U(std::forward<A0>(a0), std::forward<A1>(a1)); }
        template< class U, class A0, class A1, class A2 >
            void construct(U* p, A0&& a0, A1&& a1, A2&& a2) { new ((void*)p) U(std::forward<A0>(a0), std::forward<A1>(a1),
                    std::forward<A2>(a2)); }
        template< class U, class A0, class A1, class A2, class A3 >
            void construct(U* p, A0&& a0, A1&& a1, A2&& a2, A3&& a3) { new ((void*)p) U(std::forward<A0>(a0), std::forward<A1>(a1),
                    std::forward<A2>(a2), std::forward<A3>(a3)); }
        template< class U, class A0, class A1, class A2, class A3, class A4 >
            void construct(U* p, A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4) { new ((void*)p) U(std::forward<A0>(a0), std::forward<A1>(a1),
                    std::forward<A2>(a2), std::forward<A3>(a3), std::forward<A4>(a4)); }
#else
        template< class U, class... Args >
            void construct(U* p, Args&&... args) { new ((void*)p) U(std::forward<Args>(args)...); }
#endif
        void destroy(pointer p) { (void)p; reinterpret_cast<T*>(p)->~T(); }

        template<class U>
            struct rebind { typedef MemPoolSTLAlloc<U, PoolId, Align> other; };

        bool operator==(const MemPoolSTLAlloc&) const { return true; }
        bool operator!=(const MemPoolSTLAlloc&) const { return false; }
    };

}

#endif

