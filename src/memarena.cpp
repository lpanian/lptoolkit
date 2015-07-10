#include "toolkit/memarena.hh"
#include <cstring>
#include "toolkit/mathcommon.hh"
#include "toolkit/mem.hh"

namespace lptk
{
    MemArena::MemArena(size_t blockSize)
        : m_blockSize(lptk::Max(blockSize, lptk::AlignValue<size_t>(sizeof(BlockHeader), 16) + 16))
        , m_pool(lptk::MEMPOOL_General)
        , m_root(nullptr)
        , m_cur(nullptr)
        , m_pos(0)
    {
    }

    MemArena::~MemArena()
    {
        BlockHeader* cur = m_root;
        while(cur)
        {
            BlockHeader* next = cur->m_next;
            mem_free(cur);
            cur = next;
        }
    }
        
    MemArena::MemArena(MemArena&& other)
        : m_blockSize(other.m_blockSize)
        , m_pool(other.m_pool)
        , m_root(other.m_root)
        , m_cur(other.m_cur)
        , m_pos(other.m_pos)
    {
        other.m_root = nullptr;
        other.m_cur = nullptr;
        other.m_pos = 0;
    }

    MemArena& MemArena::operator=(MemArena&& other)
    {
        m_blockSize = other.m_blockSize;
        m_pool = other.m_pool;
        m_root = other.m_root;
        m_cur = other.m_cur;
        m_pos = other.m_pos;

        other.m_root = nullptr;
        other.m_cur = nullptr;
        other.m_pos = 0;

        return *this;
    }
        
    MemArena::BlockHeader* MemArena::MakeBlock(size_t numBytes) const
    {
        const size_t computedSize = lptk::Max(numBytes + lptk::AlignValue<size_t>(sizeof(BlockHeader), 16),
                m_blockSize);
        char* newMem = reinterpret_cast<char*>(mem_allocate(computedSize, m_pool, 16));
        BlockHeader* newHeader = reinterpret_cast<BlockHeader*>(newMem);
        newHeader->m_next = nullptr;
        newHeader->m_size = computedSize - lptk::AlignValue<size_t>(sizeof(BlockHeader), 16);
        return newHeader;
    }

    void* MemArena::Alloc(size_t numBytes)
    {
        numBytes = lptk::AlignValue<size_t>(numBytes, 16u);

        if(!m_cur)
        {
            BlockHeader* newHeader = MakeBlock(numBytes);
            m_root = newHeader;
            m_cur = newHeader;
            m_pos = 0;
        }
        else if((m_cur->m_size - m_pos) < numBytes)
        {
            if(m_cur->m_next)
            {
                if(m_cur->m_next->m_size < numBytes)
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
        
    char* MemArena::CopyString(const char* src)
    {
        size_t const numBytes = strlen(src) + 1;
        char* dest = reinterpret_cast<char*>(Alloc(numBytes));
        strcpy(dest, src);
        return dest;
    }
    
    void MemArena::Clear()
    {
        m_pos = 0;
        m_cur = m_root;
    }
}

