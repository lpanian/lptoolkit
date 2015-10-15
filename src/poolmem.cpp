#include "toolkit/poolmem.hh"

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
PoolAlloc::PoolAlloc()
	: m_itemsPerBlock(0)
	, m_itemSize(0)
	, m_block(nullptr)
	, m_next(nullptr)
	, m_canGrow(false)
	, m_poolId(MEMPOOL_General)
{
}

void PoolAlloc::Init(int itemsPerBlock, int itemSize, bool canGrow, MemPoolId poolId)
{
	ASSERT(m_itemsPerBlock == 0 && m_itemSize == 0 && !m_block && !m_next);
	m_itemsPerBlock = itemsPerBlock;
	m_itemSize = (itemSize+15) & ~15;
	m_canGrow = canGrow;
	m_poolId = poolId;
	
	// always reserve at least one block
	Block* block = AllocBlock();
	block->m_next = nullptr;
	m_block = block;
	m_next = block->First();
}

PoolAlloc::PoolAlloc(int itemsPerBlock, int itemSize, bool canGrow, MemPoolId poolId)
	: m_itemsPerBlock(0)
	, m_itemSize(0)
	, m_block(nullptr)
	, m_next(nullptr)
	, m_canGrow(false)
	, m_poolId(MEMPOOL_General)
{
	Init(itemsPerBlock, itemSize, canGrow, poolId);
}

PoolAlloc::PoolAlloc(PoolAlloc&& other)
	: m_itemsPerBlock(0)
	, m_itemSize(0)
	, m_block(nullptr)
	, m_next(nullptr)
	, m_canGrow(false)
{
	std::swap(m_itemsPerBlock, other.m_itemsPerBlock);
	std::swap(m_itemSize, other.m_itemSize);
	std::swap(m_block, other.m_block);
	std::swap(m_next, other.m_next);
	std::swap(m_canGrow, other.m_canGrow);
}

PoolAlloc::~PoolAlloc()
{
	Block* block = m_block;
	while(block) {
		Block* next = block->m_next;
		delete[] (char*)block;
		block = next;
	}
}	

PoolAlloc::Block* PoolAlloc::AllocBlock()
{
	constexpr unsigned int blockSize = (sizeof(Block)+15u) & ~15u;
    const auto allocSize = blockSize + m_itemsPerBlock * m_itemSize;
	char* data = new (m_poolId, 16) char[allocSize];
	auto block = reinterpret_cast<Block*>(data);
	bzero(block, allocSize);
	for(int i = 0; i < m_itemsPerBlock - 1; ++i)
	{
		const auto offset = blockSize + i * m_itemSize;
		BlockItem *item = reinterpret_cast<BlockItem*>(data + offset);
		item->m_next = reinterpret_cast<BlockItem*>(data + offset + m_itemSize);
	}
	return block;
}

void* PoolAlloc::Alloc()
{
	if(!m_next)
	{
		if(m_canGrow)
		{
			Block* block = AllocBlock();
			block->m_next = m_block;
			m_block = block;
			m_next = block->First();
		}
		else
			return nullptr;
	}

	void* result = reinterpret_cast<void*>(m_next);
	m_next = m_next->m_next;
    ASSERT(IsValidPtr(result));
	return result;
}
    
bool PoolAlloc::IsValidPtr(const void* p) const
{
    Block* cur = m_block;
	while(cur) 
    {
        const auto blockStart = reinterpret_cast<size_t>(p);
        const auto blockEnd = blockStart + m_itemSize;
        const auto start = reinterpret_cast<size_t>(cur->First());
        const auto end = reinterpret_cast<size_t>(cur->First()) + m_itemsPerBlock * m_itemSize;

        if (blockStart >= start && blockEnd <= end)
            return true;
		cur = cur->m_next;
	}
    return false;
}


void PoolAlloc::Free(void* ptr)
{
    ASSERT(IsValidPtr(ptr));
	if(!ptr) return;
	BlockItem* item = reinterpret_cast<BlockItem*>(ptr);
	item->m_next = m_next;
	m_next = item;
}

}

