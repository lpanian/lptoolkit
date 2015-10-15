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

	template< class T, typename... ArgTypes> 
	T* Create(ArgTypes&&... arguments) {
		ASSERT((int)sizeof(T) <= m_itemSize);
		void* ptr = Alloc();
		if(!ptr) return nullptr;
		return new (ptr) T(std::forward<ArgTypes>(arguments)...);
	}

	template<class T> void Destroy(T* obj) {
		if(!obj) return;
		ASSERT((int)sizeof(T) <= m_itemSize);
		obj->~T();
		Free(obj);
	}
   
    // for debugging
    bool IsValidPtr(const void*) const;
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
