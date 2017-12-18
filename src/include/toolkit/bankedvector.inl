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
    inline bool BankedVector<T>::ensure_chunk_allocated(size_t index)
    {
        while (index >= m_numChunks)
        {
            if (!expand_chunks())
                return false;
        }
        return true;
    }
        
    template<typename T>
    bool BankedVector<T>::uninitialized_insert_n(size_t index, size_t n)
    {
        const auto index_offset = chunk_index_offset(m_size + n - 1); 
        if (ensure_chunk_allocated(index_offset.first))
        {
            m_size += n;
            for (size_t i = index; i < m_size - n; ++i)
            {
                const auto cur = m_size - 1 - (i - index);
                const auto from = cur - n;
                auto& curRef = this->at(cur);
                auto& fromRef = this->at(from);
                new (&curRef) T(std::move(fromRef));
                fromRef.~T();
            }
            return true;
        }
        else
        {
            return false;
        }
    }
        
    template<typename T>
    template<typename... Arg>
    bool BankedVector<T>::emplace_back(Arg&&... args)
    {
        const auto index_offset = chunk_index_offset(m_size);
        if (ensure_chunk_allocated(index_offset.first))
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
        const auto index_offset = chunk_index_offset(m_size);
        if (ensure_chunk_allocated(index_offset.first))
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
        const auto index_offset = chunk_index_offset(m_size);
        if (ensure_chunk_allocated(index_offset.first))
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
    bool BankedVector<T>::insert(size_t index, const T& v)
    {
        if (uninitialized_insert_n(index, 1))
        {
            const auto index_offset = chunk_index_offset(index);
            new (&m_chunks[index_offset.first][index_offset.second]) T(v);
            return true;
        }
        return false;
    }

    template<typename T>
    bool BankedVector<T>::insert(size_t index, T&& v)
    {
        if (uninitialized_insert_n(index, 1))
        {
            const auto index_offset = chunk_index_offset(index);
            new (&m_chunks[index_offset.first][index_offset.second]) T(std::move(v));
            return true;
        }
        return false;
    }
        
    template<typename T>
    template<typename... Arg>
    bool BankedVector<T>::emplace(size_t index, Arg&&... args)
    {
        if (uninitialized_insert_n(index, 1))
        {
            const auto index_offset = chunk_index_offset(index);
            new (&m_chunks[index_offset.first][index_offset.second]) T(std::forward<Arg>(args)...);
            return true;
        }
        return false;
    }
        
    template<typename T>
    bool BankedVector<T>::insert(iterator it, const T& v)
    {
        return insert(it - begin(), v);
    }
    
    template<typename T>
    bool BankedVector<T>::insert(iterator it, T&& v)
    {
        return insert(it - begin(), std::move(v));
    }
        
    template<typename T>
    bool BankedVector<T>::erase(size_t index)
    {
        if (index >= size())
            return false;

        for (auto cur = index; cur < m_size - 1; ++cur)
        {
            const auto index_offset = chunk_index_offset(cur);
            m_chunks[index_offset.first][index_offset.second].~T();
            const auto next_offset = chunk_index_offset(cur + 1);
            auto& next = m_chunks[next_offset.first][next_offset.second];
            new (&m_chunks[index_offset.first][index_offset.second]) T(std::move(next));
        }

        pop_back();
        return true;
    }

    template<typename T>
    bool BankedVector<T>::erase(iterator it)
    {
        return erase(it - begin());
    }

    template<typename T>
    inline void BankedVector<T>::pop_back()
    {
        ASSERT(m_size > 0);
        const auto index_offset = chunk_index_offset(--m_size);
        m_chunks[index_offset.first][index_offset.second].~T();
    }

    template<typename T>
    inline T& BankedVector<T>::back()
    {
        ASSERT(m_size > 0);
        const auto index_offset = chunk_index_offset(m_size - 1);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }

    template<typename T>
    inline const T& BankedVector<T>::back() const
    {
        ASSERT(m_size > 0);
        const auto index_offset = chunk_index_offset(m_size - 1);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }

    template<typename T>
    inline T& BankedVector<T>::operator[](size_t index)
    {
        return this->at(index);
    }

    template<typename T>
    inline const T& BankedVector<T>::operator[](size_t index) const
    {
        return this->at(index);
    }
        
    template<typename T>
    inline T& BankedVector<T>::at(size_t index)
    {
        ASSERT(index < m_size);
        const auto index_offset = chunk_index_offset(index);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }

    template<typename T>
    inline const T& BankedVector<T>::at(size_t index) const
    {
        ASSERT(index < m_size);
        const auto index_offset = chunk_index_offset(index);
        ASSERT(index_offset.first < m_numChunks);
        return m_chunks[index_offset.first][index_offset.second];
    }
        
    template<typename T>
    bool BankedVector<T>::resize(size_t newSize, const value_type& def)
    {
        if (m_size == newSize)
            return true;

        // shrinking case
        for (auto cur = newSize; cur < m_size; ++cur)
        {
            const auto index_offset = chunk_index_offset(cur);
            m_chunks[index_offset.first][index_offset.second].~T();
        }

        // growing case
        if (newSize > m_size && !reserve(newSize))
            return false;

        for (auto cur = m_size; cur < newSize; ++cur)
        {
            const auto index_offset = chunk_index_offset(cur);
            new (&m_chunks[index_offset.first][index_offset.second]) T(def);
        }

        m_size = newSize;
        return true;
    }

    template<typename T>
    bool BankedVector<T>::reserve(size_t newCapacity)
    {
        if (newCapacity <= capacity())
            return true;
        if (newCapacity > max_capacity())
            return false;
        const auto index_offset = chunk_index_offset(newCapacity);
        return ensure_chunk_allocated(index_offset.first);
    }
    
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    template<bool IsConst> 
    class BankedVector<T>::base_iterator
    {
    public:
        using BankedVectorType = std::conditional_t<IsConst, const BankedVector<T>, BankedVector<T>>;
    private:
        BankedVectorType* m_vec = nullptr;
        size_t m_curIndex = 0;
    public:
        using value_type = T;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using difference_type = ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        base_iterator() = default;
        base_iterator(BankedVectorType* bv, size_t curIndex = 0)
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

        base_iterator& operator++()
        {
            ++m_curIndex;
            return *this;
        }

        base_iterator operator++(int) 
        {
            auto tmp = *this;
            ++m_curIndex;
            return tmp;
        }

        base_iterator& operator--()
        {
            if(m_curIndex > 0 )
                --m_curIndex;
            return *this;
        }

        base_iterator operator--(int) 
        {
            auto tmp = *this;
            if(m_curIndex > 0 )
                --m_curIndex;
            return tmp;
        }

        // random access methods
        base_iterator& operator+=(difference_type n)
        {
            if(n > 0)
                m_curIndex += n;
            else if (difference_type(m_curIndex) >= -n)
                m_curIndex -= n;
            return *this;
        }

        base_iterator operator+(difference_type n) const
        {
            auto tmp = *this;
            return tmp += n;
        }

        friend base_iterator operator+(difference_type n, const base_iterator& it) 
        {
            auto tmp = it;
            return tmp += n;
        }

        base_iterator& operator-=(difference_type n)
        {
            return (*this) += -n;
        }

        base_iterator operator-(difference_type n) const
        {
            auto tmp = *this;
            return tmp -= n;
        }

        difference_type operator-(const base_iterator& other) const
        {
            return m_curIndex - other.m_curIndex;
        }

        reference operator[](difference_type n) const
        {
            return *((*this) + n);
        }

        bool operator<(const base_iterator& other) const
        {
            return m_curIndex < other.m_curIndex;
        }

        bool operator>(const base_iterator& other) const
        {
            return other < (this);
        }

        bool operator>=(const base_iterator& other) const
        {
            return !((this) < other);
        }
        
        bool operator<=(const base_iterator& other) const
        {
            return !((this) > other);
        }
        
    };
}
