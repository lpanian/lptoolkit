#pragma once

#include <type_traits>
#include "toolkit/mem/allocator.hh"

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    /*

    - this is a really simple structure one step away from just having a
      fixed size array. There's still a fixed size maximum, but it grows to
      fill that maximum by allocating pages. There's a fixed number of page
      pointers, and each page is a fixed size - but they are allocated as
      required. 
    
    - It's grow-only - once the memory is allocated it does not free it until
      the vector is destroyed.

    - It's configurable on construction, which is an advantage over 
      compile time fixed sizes.

    - The point of this is to allow some flexibility and have crazy maximums
      without actually using lots of memory when you don't need it. The idea
      would be to configure your chunk sizes for "normal" uses, and allow
      for some amount of overflow.

    */

    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    class BankedVector
    {
    public:
        BankedVector(size_t chunkLength, size_t maxNumChunks, mem::Allocator* alloc);
        BankedVector(const BankedVector&) = delete;
        BankedVector& operator=(const BankedVector&) = delete;
        BankedVector(BankedVector&&);
        BankedVector& operator=(BankedVector&&);
        ~BankedVector();

        bool empty() const { return m_size == 0; }

        template<typename... Args>
        bool emplace_back(Args&&... args);
        bool push_back(const T&);
        bool push_back(T&&);

        void pop_back();
        T& back();
        const T& back() const;

        size_t size() const { return m_size; }
        size_t capacity() const { return m_numChunks * m_chunkLength; }
        size_t max_capacity() const { m_maxNumChunks * m_chunkLength; }

        // do not do memcpy like operators on the addresses of the results of thise
        // operators. The storage is not necessarily contiguous.
        T& operator[](size_t index);
        const T& operator[](size_t index) const;

        template<bool IsConst>
        class base_iterator;

        using iterator = base_iterator<false>;
        using const_iterator = base_iterator<true>;

        iterator begin() { return iterator{ this, 0 }; }
        iterator end() { return iterator{ this, m_size; } }
        const_iterator begin() const { return iterator{ this, 0 }; }
        const_iterator end() const { return iterator{ this, m_size }; }

    private:
        void destroy();
        void move_from(BankedVector&& other);
        std::pair<size_t, size_t> chunk_index_offset(size_t index) const;
        bool expand_chunks();
        bool ensure_size(size_t index);

        mem::Allocator* m_alloc = nullptr;
        size_t m_chunkLength = 0;
        size_t m_maxNumChunks = 0;

        T** m_chunks = nullptr;
        size_t m_numChunks = 0;
        size_t m_size = 0;
    };

}

#include "bankedvector.inl"
