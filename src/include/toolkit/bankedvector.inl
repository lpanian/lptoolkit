#pragma once

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    BankedVector<T>::BankedVector(size_t chunkLength, size_t maxNumChunks, mem::Allocator* alloc)
        : m_alloc(alloc)
        , m_chunkLength(chunkLength)
        , m_maxNumChunks(maxNumChunks)
    {
    }

    template<typename T>
    BankedVector<T>::~BankedVector()
    {
        destroy();
    }
        
    template<typename T>
    void BankedVector<T>::destroy()
    {
        if (!std::is_trivially_destructible_v<T>)
        {
            size_t curChunkOffset = 0;
            for (size_t curChunk = 0; curChunk < m_numChunks; ++curChunk)
            {
                auto chunk = m_chunks[curChunk];
                const size_t chunkSize = lptk::Min(m_chunkLength, m_size - curChunkOffset);
                for (size_t curOffset = 0; curOffset < chunkSize; ++curOffset)
                    chunk[curOffset].~T();
                curChunkOffset += m_chunkLength;
            }
        }

        for (size_t i = 0; i < m_numChunks; ++i)
            m_alloc->Free(m_chunks[m_numChunks - 1 - i]);
        m_alloc->DestroyN(m_maxNumChunks, m_chunks);
    }
        
    template<typename T>
    BankedVector<T>::BankedVector(BankedVector&& other)
    {
        move_from(std::move(other));
    }

    template<typename T>
    BankedVector<T>& BankedVector<T>::operator=(BankedVector&& other)
    {
        if (this != &other)
        {
            move_from(std::move(other));
        }
        return *this;
    }
        
    template<typename T>
    void BankedVector<T>::move_from(BankedVector&& other)
    {
        destroy();
        m_alloc = other.m_alloc;
        m_chunkLength = other.m_chunkLength;
        m_maxNumChunks = other.m_maxNumChunks;

        m_chunks = std::exchange(other.m_chunks, nullptr);
        m_numChunks = std::exchange(other.m_numChunks, 0);
        m_size = std::exchange(other.m_size, 0);
    }
        
    template<typename T>
    inline std::pair<size_t, size_t> BankedVector<T>::chunk_index_offset(size_t index) const
    {
        auto result = std::make_pair(index / m_chunkLength, index % m_chunkLength);
        return result;
    }
        
    template<typename T>
    bool BankedVector<T>::expand_chunks()
    {
        if (m_numChunks == m_maxNumChunks)
            return false;

        if(!m_chunks)
            m_chunks = m_alloc->CreateN<T*>(m_maxNumChunks);

        m_chunks[m_numChunks] = reinterpret_cast<T*>(m_alloc->Alloc(sizeof(T) * m_chunkLength, alignof(T)));
        if (m_chunks[m_numChunks])
        {
            ++m_numChunks;
            return true;
        }
        else
            return false;
    }
        
    template<typename T>
    inline bool BankedVector<T>::ensure_size(size_t index)
    {
        if (index >= m_numChunks)
        {
            return expand_chunks();
        }
        return true;
    }
        
    template<typename T>
        template<typename... Arg>
        bool BankedVector<T>::emplace_back(Arg&&... args)
        {
            auto index_offset = chunk_index_offset(m_size);
            if (ensure_size(index_offset.first))
            {
                ++m_size;
                new (&m_chunks[index_offset.first][index_offset.second]) T(std::forward<Arg>(args)...);
                return true;
            }
            else
            {
                return false;
            }
        }

    template<typename T>
    bool BankedVector<T>::push_back(const T& v)
    {
        auto index_offset = chunk_index_offset(m_size);
        if (ensure_size(index_offset.first))
        {
            ++m_size;
            new (&m_chunks[index_offset.first][index_offset.second]) T(v);
            return true;
        }
        else
        {
            return false;
        }
    }

    template<typename T>
    bool BankedVector<T>::push_back(T&& v)
    {
        auto index_offset = chunk_index_offset(m_size);
        if (ensure_size(index_offset.first))
        {
            ++m_size;
            new (&m_chunks[index_offset.first][index_offset.second]) T(std::move(v));
            return true;
        }
        else
        {
            return false;
        }
    }

    template<typename T>
    inline void BankedVector<T>::pop_back()
    {
        ASSERT(m_size > 0);
        auto index_offset = chunk_index_offset(--m_size);
        m_chunks[index_offset.first][index_offset.second].~T();
    }

    template<typename T>
    inline T& BankedVector<T>::back()
    {
        ASSERT(m_size > 0);
        auto index_offset = chunk_index_offset(m_size - 1);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }

    template<typename T>
    inline const T& BankedVector<T>::back() const
    {
        ASSERT(m_size > 0);
        auto index_offset = chunk_index_offset(m_size - 1);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }

    template<typename T>
    inline T& BankedVector<T>::operator[](size_t index)
    {
        ASSERT(index < m_size);
        auto index_offset = chunk_index_offset(m_size - 1);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }

    template<typename T>
    inline const T& BankedVector<T>::operator[](size_t index) const
    {
        ASSERT(index < m_size);
        auto index_offset = chunk_index_offset(m_size - 1);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    template<bool IsConst> 
    class BankedVector<T>::base_iterator
    {
        BankedVector<T>* m_vec = nullptr;
        size_t m_curIndex = 0;
    public:
        using value_type = T;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using difference_type = ptrdiff_t;

        base_iterator() = default;
        base_iterator(BankedVector<T>* bv, size_t curIndex = 0)
            : m_vec(bv)
            , m_curIndex(curIndex)
        {
        }
        base_iterator(const base_iterator&) = default;
        base_iterator& operator=(const base_iterator&) = default;
        base_iterator(base_iterator&&) = default;
        base_iterator& operator=(base_iterator&&) = default;

        bool operator==(const base_iterator& other) const
        {
            return m_vec == other.m_vec && m_curIndex == other.m_curIndex;
        }

        bool operator!=(const base_iterator& other) const
        {
            return !operator==(other);
        }

        reference operator*() const {
            return (*m_vec)[m_curIndex];
        }
        pointer operator->() const {
            return &(*m_vec)[m_curIndex];
        }

        // std::conditional<IsConst>
        base_iterator& operator++()
        {
            next();
            return *this;
        }

        base_iterator operator++(int) 
        {
            auto tmp = *this;
            next();
            return tmp;
        }

        base_iterator& operator--()
        {
            prev();
            return *this;
        }

        base_iterator operator--(int) 
        {
            auto tmp = *this;
            prev();
            return tmp;
        }
    private:
        void next()
        {
            ++m_curIndex;
        }

        void prev()
        {
            if (m_curIndex > 0)
                --m_curIndex;
        }
    };
}
