#pragma once
#ifndef INCLUDED_toolkit_dynary_hh
#define INCLUDED_toolkit_dynary_hh

#include <cstring>
#include <type_traits>
#include "toolkit/mathcommon.hh"

////////////////////////////////////////////////////////////////////////////////
// DynAry is a vector-like structure with realloc'd growing capacity.
template<class T>
class DynAry
{
public:
	typedef T* iterator;
	typedef const T* const_iterator;
	typedef uint32_t size_type;
private:
	typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type StorageType;
	StorageType* m_array;
	size_type m_capacity;
	size_type m_size;
	float m_grow;
	MemPoolId m_pool;
public:
	////////////////////////////////////////	
	// ctors
	explicit DynAry(MemPoolId pool = MEMPOOL_General) 
		: m_array(nullptr)
		, m_capacity(0)
		, m_size(0)
		, m_grow(1.5f)
		, m_pool(pool)
	{
	}

	DynAry(size_type initialSize, const T& init, MemPoolId pool = MEMPOOL_General) 
		: m_array(nullptr)
		, m_capacity(initialSize)
		, m_size(initialSize)
		, m_grow(1.5f)
		, m_pool(pool)
	{
		internal_ctor_init(initialSize, init, pool);
	}

	explicit DynAry(size_type initialSize, MemPoolId pool = MEMPOOL_General) 
		: m_array(nullptr)
		, m_capacity(initialSize)
		, m_size(initialSize)
		, m_grow(1.5f)
		, m_pool(pool)
	{
		if(m_capacity > 0)
			m_array = new (pool, alignof(T)) StorageType[initialSize];
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = 0, c = m_size; i < c; ++i)
			new (&objs[i]) T();
	}

	template<class Iter>
	DynAry(Iter begin, Iter end, MemPoolId pool = MEMPOOL_General)
		: m_array(nullptr)
		, m_capacity(end - begin)
		, m_size(end - begin)
		, m_grow(1.5f)
		, m_pool(pool)
	{
		internal_ctor_init(begin, end, pool);
	}
	
	DynAry(const DynAry& other) 
		: m_array(nullptr)
		, m_capacity(other.m_capacity)
		, m_size(other.m_size)
		, m_grow(other.m_grow)
		, m_pool(other.m_pool)
	{
		m_array = new(m_pool, alignof(T)) StorageType[m_capacity];
		T* dest = reinterpret_cast<T*>(m_array);
		const T* src = reinterpret_cast<T*>(other.m_array);
		for(size_type i = 0, c = m_size; i < c; ++i)
			new (&dest[i]) T(src[i]);
	}
	
	DynAry(const DynAry& other, MemPoolId pool) 
		: m_array(nullptr)
		, m_capacity(other.m_capacity)
		, m_size(other.m_size)
		, m_grow(other.m_grow)
		, m_pool(pool)
	{
		m_array = new(pool, alignof(T)) StorageType[m_capacity];
		T* dest = reinterpret_cast<T*>(m_array);
		const T* src = reinterpret_cast<T*>(other.m_array);
		for(size_type i = 0, c = m_size; i < c; ++i)
			new (&dest[i]) T(src[i]);
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
		T* objs = reinterpret_cast<T*>(m_array);
		(void)objs;
		for(size_type i = 0, c = m_size; i < c; ++i)
			objs[i].~T();
		delete[] m_array;
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

	void assign(size_type count, const T& value) {
		clear();
		reserve(count);
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = 0, c = count; i < c; ++i)
			new (&objs[i]) T(value);
		m_size = count;
	}

	template<class Iter>
	void assign(Iter first, Iter last) {
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

		DynAry newBuffer(m_pool);

		StorageType* storage = new (m_pool, alignof(T)) StorageType[newCapacity];

		T* destObj = reinterpret_cast<T*>(storage);
		T* srcObj = reinterpret_cast<T*>(m_array);
		size_type newSize = Min(m_size, newCapacity);
		for(size_type i = 0, c = newSize; i < c; ++i)
			new (&destObj[i]) T(std::move(srcObj[i]));

		// delete old stuff
		for(size_type i = 0, c = m_size; i < c; ++i)
			srcObj[i].~T();
		delete[] m_array;
	
		// update this object
		m_array = storage;
		m_size = newSize;
		m_capacity = newCapacity;
	}

	void resize(size_type newSize, const T& initVal)
	{
		T* objs = reinterpret_cast<T*>(m_array);
		if( newSize > m_size )
			insert(end(), newSize - m_size, initVal);
		else 
		{
			for(size_type i = newSize, c = m_size; i < c; ++i)
				objs[i].~T();
			m_size = newSize;
		}
	}
	
	void resize(size_type newSize)
	{
		if( newSize > m_size )
			insert_default_n(end(), newSize - m_size);
		else 
		{
			T* objs = reinterpret_cast<T*>(m_array);
			(void)objs;
			for(size_type i = newSize, c = m_size; i < c; ++i)
				objs[i].~T();
			m_size = newSize;
		}
	}

	void reserve(size_type newCapacity)
	{
		if(newCapacity > m_capacity)
		{
			StorageType *storage = new(m_pool, alignof(T)) StorageType[newCapacity];

			T* dest = reinterpret_cast<T*>(storage);
			T* src = reinterpret_cast<T*>(m_array);
			for(size_type i = 0, c = m_size; i < c; ++i)
				new (&dest[i]) T(std::move(src[i]));
			for(size_type i = 0, c = m_size; i < c; ++i)
				src[i].~T();
			delete[] m_array;

			m_array = storage;
			m_capacity = newCapacity;
			m_size = m_size;
		}
	}

	////////////////////////////////////////	
	// modifying 
	void clear()
	{
		T* objs = reinterpret_cast<T*>(m_array);
		(void)objs;
		for(size_type i = 0, c = m_size; i < c; ++i)
			objs[i].~T();
		m_size = 0;
	}

	void insert(const_iterator pos, const T& v)
	{
		size_type insertIndex = uninitialized_insert_n(pos, 1);
		fill_n(insertIndex, 1, v);
	}
	
	void insert(const_iterator pos, T&& v)
	{
		const size_type insertIndex = uninitialized_insert_n(pos, 1);
		T* objs = reinterpret_cast<T*>(m_array);
		new (&objs[insertIndex]) T(std::move(v));
	}

	void insert(const_iterator pos, size_type n, const T& v)
	{
		ASSERT(pos <= begin() + m_size);
		if(n == 0)
			return;

		size_type insertIndex = uninitialized_insert_n(pos, n);
		fill_n(insertIndex, n, v);
	}
	
	template<class In>
	void insert(const_iterator pos, In first, In last)
	{
		static_assert(!std::is_integral<In>::value, "wrong insert function being called");
		ASSERT(last >= first);
		size_type n = (size_type)(last - first);
		if(n == 0)
			return;
		size_type insertIndex = uninitialized_insert_n(pos, n);
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = insertIndex, last = insertIndex+n; i < last; ++first, ++i)
			new	(&objs[i]) T(*first);
	}

	void erase(const_iterator pos)
	{
		ASSERT(pos >= begin() && pos < end());
		ASSERT(!empty());

		iterator cursor = (begin() + (pos - begin())) + 1;
		while(cursor != end())
		{
			std::swap(*(cursor - 1), *cursor);
			++cursor;
		}
		--m_size;
	}

	void erase(const_iterator first, const_iterator last)
	{
		ASSERT(last >= first);
		ASSERT(first >= begin() && last <= end() && "bad iterator range");

		size_type amountToCut = (size_type)(last - first);
		if(amountToCut == 0)
			return;

		ASSERT(amountToCut < m_size && "Invalid range"); 

		size_type cutIndex = (size_type)(first-begin());
		size_type lastIndex = m_size - amountToCut;
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = cutIndex; i < lastIndex; ++i) 
		{
			new(&objs[i]) T(std::move(objs[i+amountToCut]));
			objs[i+amountToCut].~T();
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
		size_type insertIndex = uninitialized_insert_n(end(), 1);
		fill_n(insertIndex, 1, v);
	}
	
	void push_back(T&& v)
	{
		size_type insertIndex = uninitialized_insert_n(end(), 1);
		move_1(insertIndex, std::move(v));
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

	T& operator[](size_type index) { ASSERT(index < m_size); return ObjPtr()[index]; }
	const T& operator[](size_type index) const { ASSERT(index < m_size); return ObjPtr()[index]; }

	T& at(size_type index) { ASSERT(index < m_size); return ObjPtr()[index]; }
	const T& at(size_type index) const { ASSERT(index < m_size); return ObjPtr()[index]; }

	T& front() { ASSERT(m_size > 0); return ObjPtr()[0]; }
	const T& front() const { ASSERT(m_size > 0); return ObjPtr()[0]; }
	T& back() { ASSERT(m_size > 0); return ObjPtr()[m_size-1]; }
	const T& back() const { ASSERT(m_size > 0); return ObjPtr()[m_size-1]; }

	iterator begin() { return &(ObjPtr()[0]); }
	const_iterator begin() const { return &(ObjPtr()[0]); }
	iterator end() { return &(ObjPtr()[m_size]); }
	const_iterator end() const { return &(ObjPtr()[m_size]); }

private:
	////////////////////////////////////////
	// Compile time choice of contructor implementation based on whether or not T is integral.
	// this avoids the dynary<int>(10, 1) vs. dynary<int>(pIntAry, pIntAry+10) problem. Apparently std::vector
	// does this too. 
	void internal_ctor_init(size_type initialSize, const T& init, MemPoolId pool)
	{
		if(m_capacity > 0)
			m_array = new (pool, alignof(T)) StorageType[initialSize];
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = 0, c = m_size; i < c; ++i)
			new (&objs[i]) T(init);
	}
	
	template<class Iter>
	typename std::enable_if<!std::is_integral<Iter>::value, void>::type
	internal_ctor_init(Iter begin, Iter end, MemPoolId pool)
	{
		m_array = new(pool, alignof(T)) StorageType[m_capacity];
		Iter cur = begin;
		int i = 0;
		T* objs = reinterpret_cast<T*>(m_array);
		for(; cur != end; ++cur, ++i)
			new (&objs[i]) T(*cur);
	}

	////////////////////////////////////////
	T* ObjPtr() { return reinterpret_cast<T*>(m_array); }
	const T* ObjPtr() const { return reinterpret_cast<T*>(m_array); }
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
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = m_size - 1; i > (insertIndex + n - 1); --i)
		{
			new(&objs[i]) T(std::move(objs[i-n]));
			objs[i-n].~T();
		}

		return insertIndex;
	}

	void fill_default_n(size_type index, size_type n)
	{
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = index, last = index+n; i < last; ++i)
			new	(&objs[i]) T;
	}
	
	void fill_n(size_type index, size_type n, const T& val)
	{
		T* objs = reinterpret_cast<T*>(m_array);
		for(size_type i = index, last = index+n; i < last; ++i)
			new	(&objs[i]) T(val);
	}
	
	void move_1(size_type index, T&& val)
	{
		T* objs = reinterpret_cast<T*>(m_array);
		new (&objs[index]) T(std::move(val));
	}

	void insert_default_n(const_iterator pos,  size_type n)
	{
		size_type insertIndex = uninitialized_insert_n(pos, n);
		fill_default_n(insertIndex, n);
	}
};


#endif

