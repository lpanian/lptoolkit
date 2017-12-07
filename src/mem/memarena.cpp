#include "toolkit/mem/memarena.hh"
#include <cstring>
#include "toolkit/mathcommon.hh"

namespace lptk
{
    namespace mem
    {
        MemArena::MemArena(size_t blockSize, mem::Allocator* alloc)
            : m_blockSize(lptk::Max(blockSize, lptk::AlignValue<size_t>(sizeof(BlockHeader), 16) + 16))
            , m_alloc(alloc)
        {
        }

        MemArena::~MemArena()
        {
            BlockHeader* cur = m_root;
            while (cur)
            {
                BlockHeader* next = cur->m_next;
                m_alloc->Free(cur);
                cur = next;
            }
        }
            
        void MemArena::Move(MemArena&& other)
        {
            m_blockSize = std::exchange(other.m_blockSize, 0);
            m_root = std::exchange(other.m_root, nullptr);
            m_cur = std::exchange(other.m_cur, nullptr);
            m_pos = std::exchange(other.m_pos, 0);
            m_alloc = other.m_alloc;
        }

        MemArena::MemArena(MemArena&& other)
        {
            Move(std::move(other));
        }

        MemArena& MemArena::operator=(MemArena&& other)
        {
            if (this != &other)
            {
                Move(std::move(other));
            }
            return *this;
        }

        MemArena::BlockHeader* MemArena::MakeBlock(size_t numBytes) const
        {
            const size_t computedSize = lptk::Max(numBytes + lptk::AlignValue<size_t>(sizeof(BlockHeader), 16),
                m_blockSize);
            char* newMem = reinterpret_cast<char*>(m_alloc->Alloc(computedSize, 16));
            BlockHeader* newHeader = reinterpret_cast<BlockHeader*>(newMem);
            newHeader->m_size = computedSize - lptk::AlignValue<size_t>(sizeof(BlockHeader), 16);
            return newHeader;
        }

        void* MemArena::Alloc(size_t numBytes, unsigned align)
        {
            lptk::unused_arg(align); // TODO should be able to align up to builtin alignment
            numBytes = lptk::AlignValue<size_t>(numBytes, 16u);

            if (!m_cur)
            {
                BlockHeader* newHeader = MakeBlock(numBytes);
                m_root = newHeader;
                m_cur = newHeader;
                m_pos = 0;
            }
            else if ((m_cur->m_size - m_pos) < numBytes)
            {
                if (m_cur->m_next)
                {
                    if (m_cur->m_next->m_size < numBytes)
                    {
                        BlockHeader* newHeader = MakeBlock(numBytes);
                        newHeader->m_next = m_cur->m_next;
                        m_cur->m_next = newHeader;
                        m_cur = newHeader;
                        m_pos = 0;
                    }
                    else
                    {
                        m_cur = m_cur->m_next;
                        m_pos = 0;
                    }
                }
                else
                {
                    BlockHeader* newHeader = MakeBlock(numBytes);
                    m_cur->m_next = newHeader;
                    m_cur = newHeader;
                    m_pos = 0;
                }
            }

            char* dataStart = reinterpret_cast<char*>(m_cur) + lptk::AlignValue<size_t>(sizeof(BlockHeader), 16);

            char* result = dataStart + m_pos;
            m_pos += numBytes;
            ASSERT(m_pos <= m_cur->m_size);

            return result;
        }
            
        void MemArena::Free(void*)
        {
        }

        char* MemArena::CopyString(const char* src)
        {
            size_t const numBytes = strlen(src) + 1;
            char* dest = reinterpret_cast<char*>(Alloc(numBytes, 1));
            strcpy(dest, src);
            return dest;
        }

        void MemArena::Clear()
        {
            m_pos = 0;
            m_cur = m_root;
        }
    }
}

