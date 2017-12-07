#pragma once

#include "allocator.hh"

namespace lptk
{
    namespace mem
    {
        //////////////////////////////////////////////////////////////////////////////// 
        class LinearChunkAllocator : public mem::Allocator
        {
        public:
            LinearChunkAllocator(size_t blockSize = 1 << 15, mem::Allocator* alloc = mem::GetDefaultAllocator());
            LinearChunkAllocator(const LinearChunkAllocator&) = delete;
            LinearChunkAllocator& operator=(const LinearChunkAllocator&) = delete;
            LinearChunkAllocator(LinearChunkAllocator&& other);
            LinearChunkAllocator& operator=(LinearChunkAllocator&& other);
            ~LinearChunkAllocator();

            void* Alloc(size_t numBytes, unsigned align) 
                override;
            void Free(void*) 
                override;
            
            char* CopyString(const char* src);

            void Clear();
        private:
            void Move(LinearChunkAllocator&& other);
            struct BlockHeader
            {
                BlockHeader* m_next = nullptr;
                size_t m_size = 0;
            };

            BlockHeader* MakeBlock(size_t numBytes) const;

            size_t m_blockSize = 0;
            BlockHeader* m_root = nullptr;
            BlockHeader* m_cur = nullptr;
            size_t m_pos = 0;
            mem::Allocator* m_alloc = nullptr;
        };
    }
}


