#pragma once
#ifndef INCLUDED_toolkit_poolmem_hh
#define INCLUDED_toolkit_poolmem_hh

//#include <memory>
#include <cstring>
#include <utility>
#include "common.hh"

namespace lptk
{

class PoolAlloc
{
public:
	PoolAlloc();
	PoolAlloc(int itemsPerBlock, int itemSize, bool canGrow = true, MemPoolId poolId = MEMPOOL_General);
	~PoolAlloc();

	PoolAlloc(const PoolAlloc&) DELETED;
	PoolAlloc& operator=(const PoolAlloc&) DELETED;

	PoolAlloc(PoolAlloc&& other);

	void Init(int itemsPerBlock, int itemSize, bool canGrow = true, MemPoolId poolId = MEMPOOL_General);
	void* Alloc();
	void Free(void*);

#ifdef USING_VS
	// faked variadic templates
	template< class T > 
	T* Create() {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		return ptr ? new (ptr) T() : nullptr;
	}
	template< class T, class A0 > 
	T* Create(A0&& a0) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		return ptr ? new (ptr) T(std::forward<A0>(a0)) : nullptr;
	}
	template< class T, class A0, class A1 >
	T* Create(A0&& a0, A1&& a1 ) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		return ptr ? new (ptr) T(
				std::forward<A0>(a0),
				std::forward<A1>(a1)
				) : nullptr;
	}
	template< class T, class A0, class A1, class A2 >
	T* Create(A0&& a0, A1&& a1, A2&& a2 ) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		return ptr ? new (ptr) T(
				std::forward<A0>(a0),
				std::forward<A1>(a1),
				std::forward<A2>(a2)
				) : nullptr;
	}
	template< class T, class A0, class A1, class A2, class A3 >
	T* Create(A0&& a0, A1&& a1, A2&& a2, A3&& a3 ) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		return ptr ? new (ptr) T(
				std::forward<A0>(a0),
				std::forward<A1>(a1),
				std::forward<A2>(a2),
				std::forward<A3>(a3)
				) : nullptr;
	}
	template< class T, class A0, class A1, class A2, class A3, class A4 >
	T* Create(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4 ) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		return ptr ? new (ptr) T(
				std::forward<A0>(a0),
				std::forward<A1>(a1),
				std::forward<A2>(a2),
				std::forward<A3>(a3),
				std::forward<A4>(a4)
				) : nullptr;
	}
	template< class T, class A0, class A1, class A2, class A3, class A4, class A5 >
	T* Create(A0&& a0, A1&& a1, A2&& a2, A3&& a3, A4&& a4, A5&& a5 ) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		return ptr ? new (ptr) T(
				std::forward<A0>(a0),
				std::forward<A1>(a1),
				std::forward<A2>(a2),
				std::forward<A3>(a3),
				std::forward<A4>(a4),
				std::forward<A5>(a5)
				) : nullptr;
	}
#else
	// C++11 variadic templates!
	template< class T, typename... ArgTypes> 
	T* Create(ArgTypes&&... arguments) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		if(!ptr) return nullptr;
		return new (ptr) T(std::forward<ArgTypes>(arguments)...);
	}
#endif

	template<class T> void Destroy(T* obj) {
		if(!obj) return;
		ASSERT((int)sizeof(T) <= m_itemSize);
		obj->~T();
		Free(obj);
	}
private:
	class BlockItem {
	public:
		BlockItem* m_next;
	};
	class Block { 
	public:
		Block* m_next;
		BlockItem* First() { 
			constexpr int blockSize = (sizeof(Block)+15) & ~15;
			return reinterpret_cast<BlockItem*>(
				reinterpret_cast<char*>(this) + blockSize);
		}
	};

	Block* AllocBlock();

	////////////////////////////////////////	
	int m_itemsPerBlock;
	int m_itemSize;
	Block* m_block;
	BlockItem* m_next;
	bool m_canGrow;
	MemPoolId m_poolId;
};

/*
template< class T > 
class PoolAllocSTL
{
	PoolAlloc m_pool;
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	PoolAllocSTL(int itemsPerBlock) : m_pool(itemsPerBlock, sizeof(T)) {}
	PoolAllocSTL(const PoolAllocSTL&) = delete;
	const PoolAllocSTL& operator=(const PoolAllocSTL&) = delete;

	pointer address(reference r) const { return &r; }
	const_pointer address(const_reference r) const { return &r; }
	pointer allocate(size_type n, std::allocator<void>::const_pointer hint = nullptr) {
		ASSERT(n == 1); // can only allocate one element
		if(n != 1) return nullptr;
		else return m_pool.Alloc();
	}
	void deallocate(pointer p, size_type n) {
		ASSERT(n == 1); // can only allocate one element
		if(n == 1) m_pool.Free(p);
	}
	size_type max_size() const { return 1; }
	void construct(pointer p, const_reference val) { new ((void*)p) T(val); }
	void destroy(pointer p) { ((T*)p)->~T(); }
};
*/

}

#endif
