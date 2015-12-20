#pragma once
#ifndef INCLUDED_toolkit_dynary_hh
#define INCLUDED_toolkit_dynary_hh

#include <cstring>
#include <type_traits>
#include "toolkit/mathcommon.hh"

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
// DynAry is a vector-like structure with realloc'd growing capacity.
template<class T, unsigned int ALIGN=alignof(T), MemPoolId DefaultPool = MEMPOOL_General>
class DynAry
{
public:
    using iterator = T*;
    using const_iterator = T*;
    using size_type = size_t;
private:
    T* m_array;
    size_type m_capacity;
    size_type m_size;
    float m_grow;
    MemPoolId m_pool;
public:
    ////////////////////////////////////////	
    // ctors
    explicit DynAry(MemPoolId pool = DefaultPool) 
        : m_array(nullptr)
        , m_capacity(0)
        , m_size(0)
        , m_grow(1.5f)
        , m_pool(pool)
    {
    }

    DynAry(size_type initialSize, const T& init, MemPoolId pool = DefaultPool) 
        : m_array(nullptr)
        , m_capacity(initialSize)
        , m_size(initialSize)
        , m_grow(1.5f)
        , m_pool(pool)
    {
        internal_ctor_init(init);
    }

    explicit DynAry(size_type initialSize, MemPoolId pool = DefaultPool) 
        : m_array(nullptr)
        , m_capacity(initialSize)
        , m_size(initialSize)
        , m_grow(1.5f)
        , m_pool(pool)
    {
        internal_ctor_init(T());
    }

    template<class Iter>
        DynAry(Iter begin, Iter end, MemPoolId pool = DefaultPool)
        : m_array(nullptr)
        , m_capacity(end - begin)
        , m_size(end - begin)
        , m_grow(1.5f)
        , m_pool(pool)
    {
        internal_ctor_init(begin, end);
    }

    template<class U>
    DynAry(std::initializer_list<U> init)
        : m_array(nullptr)
        , m_capacity(init.size())
        , m_size(init.size())
        , m_grow(1.5f)
        , m_pool(DefaultPool)
    {
        internal_ctor_init(init.begin(), init.end());
    }

    DynAry(const DynAry& other) 
        : m_array(nullptr)
        , m_capacity(other.m_capacity)
        , m_size(other.m_size)
        , m_grow(other.m_grow)
        , m_pool(other.m_pool)
    {
        void* p = mem_allocate(sizeof(T) * m_capacity, m_pool, ALIGN);
        m_array = reinterpret_cast<T*>(p);
        const T* src = reinterpret_cast<T*>(other.m_array);
        for(size_type i = 0, c = m_size; i < c; ++i)
            new (&m_array[i]) T(src[i]);
    }

    DynAry(const DynAry& other, MemPoolId pool) 
        : m_array(nullptr)
        , m_capacity(other.m_capacity)
        , m_size(other.m_size)
        , m_grow(other.m_grow)
        , m_pool(pool)
    {
        void* p = mem_allocate(sizeof(T) * m_capacity, m_pool, ALIGN);
        m_array = reinterpret_cast<T*>(p);
        const T* src = reinterpret_cast<T*>(other.m_array);
        for(size_type i = 0, c = m_size; i < c; ++i)
            new (&m_array[i]) T(src[i]);
    }

    DynAry(DynAry&& other)
        : m_array(nullptr)
        , m_capacity(0)
        , m_size(0)
        , m_grow(other.m_grow)
        , m_pool(other.m_pool)
    {
        std::swap(m_array, other.m_array);
        std::swap(m_capacity, other.m_capacity);
        std::swap(m_size, other.m_size);
    }

    ////////////////////////////////////////	
    ~DynAry()
    {
        for(size_type i = 0, c = m_size; i < c; ++i)
            m_array[i].~T();
        mem_free(m_array);
    }

    ////////////////////////////////////////	
    // assignment
    DynAry& operator=(const DynAry& other)
    {
        if(&other != this)
        {
            clear();
            DynAry otherCopy(other);
            swap(otherCopy);
        }
        return *this;
    }

    DynAry& operator=(DynAry&& other)
    {
        clear();
        swap(other);
        return *this;
    }

    void assign(size_type count, const T& value) 
    {
        clear();
        reserve(count);
        T* objs = reinterpret_cast<T*>(m_array);
        for(size_type i = 0, c = count; i < c; ++i)
            new (&objs[i]) T(value);
        m_size = count;
    }

    template<class Iter>
    void assign(Iter first, Iter last) 
    {
        clear();
        size_type count = last - first;
        reserve(count);
        T* objs = reinterpret_cast<T*>(m_array);
        size_type i = 0;
        Iter cur = first;
        for(; cur != last; ++i, ++cur)
            new (&objs[i]) T(*cur);
        m_size = count;
    }

    ////////////////////////////////////////	
    // capacity 
    void set_capacity(size_type newCapacity)
    {
        if(m_capacity == newCapacity)
            return;
        else if(newCapacity > m_capacity)
        {
            reserve(newCapacity);
        }
        else // newCapacity < m_capacity
        {
            for(size_t i = newCapacity; i < m_size; ++i)
                m_array[i].~T();

            const size_t newSize = Min(m_size, newCapacity);
            if(__is_trivially_copyable(T))
            //if(std::is_trivially_copyable<T>::value)
            {
                m_array = reinterpret_cast<T*>(mem_reallocate(m_array, sizeof(T) * newCapacity, m_pool, ALIGN));
            }
            else
            {
                T* newAry = reinterpret_cast<T*>(mem_allocate(sizeof(T) * newCapacity, m_pool, ALIGN));
                for(size_t i = 0, c = newSize; i < c; ++i)
                {
                    new (&newAry[i]) T(std::move(m_array[i]));
                    m_array[i].~T();
                }
                mem_free(m_array);
                m_array = newAry;
            }
            m_size = newSize;
            m_capacity = newCapacity;
        }
    }

    void resize(size_type newSize, const T& initVal)
    {
        if( newSize > m_size )
            insert(end(), newSize - m_size, initVal);
        else 
        {
            for(size_type i = newSize, c = m_size; i < c; ++i)
                m_array[i].~T();
            m_size = newSize;
        }
    }

    void resize(size_type newSize)
    {
        if( newSize > m_size )
            insert_default_n(end(), newSize - m_size);
        else 
        {
            for(size_type i = newSize, c = m_size; i < c; ++i)
                m_array[i].~T();
            m_size = newSize;
        }
    }

    void reserve(size_type newCapacity)
    {
        if(newCapacity > m_capacity)
        {
            if(__is_trivially_copyable(T))
            //if(std::is_trivially_copyable<T>::value)
            {
                m_array = reinterpret_cast<T*>(mem_reallocate(m_array, sizeof(T) * newCapacity, m_pool, ALIGN));
            }
            else
            {
                T* newPtr = reinterpret_cast<T*>(mem_allocate(sizeof(T) * newCapacity, m_pool, ALIGN));
                for(size_type i = 0, c = m_size; i < c; ++i)
                {
                    new (&newPtr[i]) T(std::move(m_array[i]));
                    m_array[i].~T();
                }
                mem_free(m_array);
                m_array = newPtr;
            }
            m_capacity = newCapacity;
        }
    }

    ////////////////////////////////////////	
    // modifying 
    void clear()
    {
        for(size_type i = 0, c = m_size; i < c; ++i)
            m_array[i].~T();
        m_size = 0;
    }

    void insert(const_iterator pos, const T& v)
    {
        const size_type insertIndex = uninitialized_insert_n(pos, 1);
        fill_n(insertIndex, 1, v);
    }

    void insert(const_iterator pos, T&& v)
    {
        const size_type insertIndex = uninitialized_insert_n(pos, 1);
        new (&m_array[insertIndex]) T(std::move(v));
    }

    void insert(const_iterator pos, size_type n, const T& v)
    {
        ASSERT(pos <= begin() + m_size);
        if(n == 0)
            return;

        const size_type insertIndex = uninitialized_insert_n(pos, n);
        fill_n(insertIndex, n, v);
    }

    template<class In>
    void insert(const_iterator pos, In first, In last)
    {
        static_assert(!std::is_integral<In>::value, "wrong insert function being called");
        ASSERT(last >= first);
        const size_type n = (size_type)(last - first);
        if(n == 0)
            return;
        const size_type insertIndex = uninitialized_insert_n(pos, n);
        for(size_type i = insertIndex, last = insertIndex+n; i < last; ++first, ++i)
            new (&m_array[i]) T(*first);
    }

    void erase(const_iterator pos)
    {
        erase(pos, pos+1);
    }

    void erase(const_iterator first, const_iterator last)
    {
        ASSERT(last >= first);
        ASSERT(first >= begin() && last <= end() && "bad iterator range");

        const size_type amountToCut = (size_type)(last - first);
        if(amountToCut == 0)
            return;

        const size_type cutIndex = (size_type)(first - begin());
        ASSERT(cutIndex + amountToCut <= m_size && "Invalid range"); 
        const size_type lastIndex = cutIndex + amountToCut;
        const size_type firstEnd = m_size - amountToCut;
        for(size_type i = cutIndex; i < lastIndex; ++i) 
        {
            m_array[i].~T();
            new(&m_array[i]) T(std::move(m_array[firstEnd + i]));
            m_array[firstEnd + i].~T();
        }
        m_size -= amountToCut;
    }

    void swap(DynAry &other)
    {
        std::swap(other.m_array, m_array);
        std::swap(other.m_capacity, m_capacity);
        std::swap(other.m_size, m_size);
        std::swap(other.m_grow, m_grow);
        std::swap(other.m_pool, m_pool);	
    }

    void push_back(const T& v)
    {
        const size_type insertIndex = uninitialized_insert_n(end(), 1);
        fill_n(insertIndex, 1, v);
    }

    void push_back(T&& v)
    {
        const size_type insertIndex = uninitialized_insert_n(end(), 1);
        move_1(insertIndex, std::move(v));
    }

    template<class... Args>
    void emplace_back(Args&&... args)
    {
        const size_type insertIndex = uninitialized_insert_n(end(), 1);
        construct_1(insertIndex, std::forward<Args>(args)...);
    }

    void pop_back()
    {
        ASSERT( m_size > 0 );
        erase(end() - 1);	
    }

    int index_of(const_iterator iter)
    {
        return (iter - begin());
    }

    size_type size() const { return m_size; }
    size_type capacity() const { return m_capacity; }
    bool empty() const { return m_size == 0; }

    T& operator[](size_type index) { ASSERT(index < m_size); return m_array[index]; }
    const T& operator[](size_type index) const { ASSERT(index < m_size); return m_array[index]; }

    T& at(size_type index) { ASSERT(index < m_size); return m_array[index]; }
    const T& at(size_type index) const { ASSERT(index < m_size); return m_array[index]; }

    const T& get(size_type index) const { ASSERT(index < m_size); return m_array[index]; }
    void set(size_type index, const T& v) { ASSERT(index < m_size); m_array[index] = v; } 

    T& front() { ASSERT(m_size > 0); return m_array[0]; }
    const T& front() const { ASSERT(m_size > 0); return m_array[0]; }
    T& back() { ASSERT(m_size > 0); return m_array[m_size-1]; }
    const T& back() const { ASSERT(m_size > 0); return m_array[m_size-1]; }

    T* data() { return &(m_array[0]); }
    const T* data() const { return &(m_array[0]); }

    iterator begin() { return &(m_array[0]); }
    const_iterator begin() const { return &(m_array[0]); }
    iterator end() { return &(m_array[m_size]); }
    const_iterator end() const { return &(m_array[m_size]); }

private:
    ////////////////////////////////////////
    // Compile time choice of contructor implementation based on whether or not T is integral.
    // this avoids the dynary<int>(10, 1) vs. dynary<int>(pIntAry, pIntAry+10) problem. Apparently std::vector
    // does this too. 
    template<class U>
    void internal_ctor_init(U&& init)
    {
        ASSERT(m_size <= m_capacity);
        if(m_capacity > 0)
            m_array = reinterpret_cast<T*>(mem_allocate(sizeof(T) * m_capacity, m_pool, ALIGN));
        for(size_type i = 0, c = m_size; i < c; ++i)
            new (&m_array[i]) T(std::forward<U>(init));
    }

    template<class Iter>
    typename std::enable_if<!std::is_integral<Iter>::value, void>::type
    internal_ctor_init(Iter begin, Iter end)
    {
        ASSERT(m_size <= m_capacity);
        m_array = reinterpret_cast<T*>(mem_allocate(sizeof(T) * m_capacity, m_pool, ALIGN));
        Iter cur = begin;
        int i = 0;
        for(; cur != end; ++cur, ++i)
            new (&m_array[i]) T(*cur);
    }

    ////////////////////////////////////////
    void grow( int minSize = 16 )
    {
        int size = static_cast<int>(m_capacity * m_grow);
        if(minSize > size)
            size = minSize;
        ASSERT(size > m_capacity);
        reserve(size);
    }

    size_type uninitialized_insert_n(const_iterator pos, size_type n)
    {
        ASSERT(pos <= begin() + m_size);

        // compute insert index before possible resize
        size_type insertIndex = (size_type)(pos - begin());	

        ASSERT(insertIndex >= 0 && insertIndex <= (size_type)(end() - begin()) && "bad insert index");
        reserve(size() + n);
        m_size += n;

        // shift stuff down if we're not pushing onto the end.
        for(size_type i = m_size - 1; i > (insertIndex + n - 1); --i)
        {
            new(&m_array[i]) T(std::move(m_array[i-n]));
            m_array[i-n].~T();
        }

        return insertIndex;
    }

    void fill_default_n(size_type index, size_type n)
    {
        for(size_type i = index, last = index+n; i < last; ++i)
            new (&m_array[i]) T;
    }

    void fill_n(size_type index, size_type n, const T& val)
    {
        for(size_type i = index, last = index+n; i < last; ++i)
            new (&m_array[i]) T(val);
    }

    void move_1(size_type index, T&& val)
    {
        new (&m_array[index]) T(std::move(val));
    }

    template<class... Args>
    void construct_1(size_type index, Args&&... args)
    {
        new (&m_array[index]) T(std::forward<Args>(args)...);
    }

    void insert_default_n(const_iterator pos,  size_type n)
    {
        const size_type insertIndex = uninitialized_insert_n(pos, n);
        fill_default_n(insertIndex, n);
    }
};

}

#endif

