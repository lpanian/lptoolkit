#include "toolkit/mem/pool_allocator.hh"

namespace lptk
{
    namespace mem
    {
        ////////////////////////////////////////////////////////////////////////////////
        PoolAlloc::PoolAlloc(mem::Allocator* alloc)
            : m_alloc(alloc)
        {}

        void PoolAlloc::Init(size_t itemsPerBlock, size_t itemSize, bool canGrow, mem::Allocator* alloc)
        {
            ASSERT(m_itemsPerBlock == 0 && m_itemSize == 0 && !m_block && !m_next);
            m_itemsPerBlock = itemsPerBlock;
            m_itemSize = (itemSize + 15) & ~15;
            m_canGrow = canGrow;
            m_alloc = alloc;

            if (!canGrow)
            {
                Block* block = AllocBlock();
                m_block = block;
                m_next = block->First();
            }
        }

        PoolAlloc::PoolAlloc(size_t itemsPerBlock, size_t itemSize, bool canGrow, mem::Allocator* alloc)
        {
            Init(itemsPerBlock, itemSize, canGrow, alloc);
        }

        void PoolAlloc::Move(PoolAlloc&& other)
        {
            m_itemsPerBlock = std::exchange(other.m_itemsPerBlock, 0);
            m_itemSize = std::exchange(other.m_itemSize, 0);
            m_block = std::exchange(other.m_block, nullptr);
            m_next = std::exchange(other.m_next, nullptr);
            m_canGrow = std::exchange(other.m_canGrow, true);
            m_alloc = other.m_alloc;
        }

        PoolAlloc::PoolAlloc(PoolAlloc&& other)
        {
            Move(std::move(other));
        }

        PoolAlloc& PoolAlloc::operator=(PoolAlloc&& other)
        {
            if (this != &other)
            {
                Move(std::move(other));
            }
            return *this;
        }

        PoolAlloc::~PoolAlloc()
        {
            Block* block = m_block;
            while (block) {
                Block* next = block->m_next;
                m_alloc->Free(block);
                block = next;
            }
        }

        PoolAlloc::Block* PoolAlloc::AllocBlock()
        {
            constexpr auto blockSize = (sizeof(Block) + 15u) & ~15u;
            const auto allocSize = blockSize + m_itemsPerBlock * m_itemSize;
            char* data = reinterpret_cast<char*>(m_alloc->Alloc(allocSize, 16));
            auto block = reinterpret_cast<Block*>(data);
            bzero(block, allocSize);
            for (size_t i = 0; i < m_itemsPerBlock - 1; ++i)
            {
                const auto offset = blockSize + i * m_itemSize;
                BlockItem *item = reinterpret_cast<BlockItem*>(data + offset);
                item->m_next = reinterpret_cast<BlockItem*>(data + offset + m_itemSize);
            }
            return block;
        }

        void* PoolAlloc::Alloc(size_t size, unsigned align)
        {
            lptk::unused_arg(align);
            if (size > m_itemSize)
                return nullptr;

            if (!m_next)
            {
                if (m_canGrow)
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
            while (cur)
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
            if (!ptr) return;
            BlockItem* item = reinterpret_cast<BlockItem*>(ptr);
            item->m_next = m_next;
            m_next = item;
        }
    }
}

