#pragma once
#ifndef INCLUDED_toolkit_memarena_HH
#define INCLUDED_toolkit_memarena_HH

namespace lptk
{
    class MemArena
    {
    public:
        MemArena(size_t blockSize = 1 << 15);
        MemArena(MemArena&& other);
        MemArena& operator=(MemArena&& other);
        ~MemArena();

        void* Alloc(size_t numBytes);
        char* CopyString(const char* src);
        
        template<class T>
        T* Alloc(size_t count = 1)
        {
            T* result = static_cast<T*>(Alloc(sizeof(T) * count));
            for(size_t i = 0; i < count; ++i) 
                new (&result[i]) T();
            return result;
        }

        template<class T, typename... Args>
        T* Alloc(size_t count, Args&&... args)
        {
            T* result = static_cast<T*>(Alloc(sizeof(T) * count));
            for(size_t i = 0; i < count; ++i) 
                new (&result[i]) T(std::forward<Args>(args)...);
            return result;
        }
        
        template<class T, typename... Args>
        T* New(Args&&... args)
        {
            T* result = static_cast<T*>(Alloc(sizeof(T)));
            new (result) T(std::forward<Args>(args)...);
            return result;
        }

        void Clear();
    private:
        MemArena(const MemArena&) DELETED;
        MemArena& operator=(const MemArena&) DELETED;

        struct BlockHeader
        {
            BlockHeader* m_next;
            size_t m_size;
        };

        BlockHeader* MakeBlock(size_t numBytes) const;

        size_t m_blockSize;
        lptk::MemPoolId m_pool;
        BlockHeader* m_root;
        BlockHeader* m_cur;
        size_t m_pos;
    };
}

#endif

