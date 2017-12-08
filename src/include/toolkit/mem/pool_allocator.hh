#pragma once

#include <cstdint>
#include "../common.hh"
#include "allocator.hh"

namespace lptk
{
    namespace mem
    {
        ////////////////////////////////////////////////////////////////////////////////
        class PoolAlloc : public mem::Allocator
        {
        public:
            PoolAlloc(mem::Allocator* alloc = mem::GetDefaultAllocator());
            PoolAlloc(size_t itemsPerBlock, size_t itemSize, bool canGrow = true,
                mem::Allocator* alloc = mem::GetDefaultAllocator());
            ~PoolAlloc();

            PoolAlloc(const PoolAlloc&) = delete;
            PoolAlloc& operator=(const PoolAlloc&) = delete;

            PoolAlloc(PoolAlloc&& other);
            PoolAlloc& operator=(PoolAlloc&& other);

            void Init(size_t itemsPerBlock, size_t itemSize, bool canGrow = true,
                mem::Allocator* alloc = mem::GetDefaultAllocator());

            void* Alloc(size_t size, unsigned align)
                override;
            void Free(void*)
                override;

            // for debugging
            bool IsValidPtr(const void*) const;
        private:
            void Destruct();
            void Move(PoolAlloc&& other);

            class BlockItem
            {
            public:
                BlockItem* m_next = nullptr;
            };

            class Block
            {
            public:
                Block* m_next = nullptr;
                BlockItem* First() const
                {
                    constexpr size_t blockSize = (sizeof(Block) + 15) & ~15;
                    return reinterpret_cast<BlockItem*>(
                        reinterpret_cast<uintptr_t>(this) + blockSize);
                }
            };

            Block* AllocBlock();

            ////////////////////////////////////////    
            size_t m_itemsPerBlock = 0;
            size_t m_itemSize = 0;
            Block* m_block = nullptr;
            BlockItem* m_next = nullptr;
            bool m_canGrow = true;
            mem::Allocator* m_alloc = nullptr;
        };
    }
}

