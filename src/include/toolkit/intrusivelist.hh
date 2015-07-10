#pragma once
#ifndef INCLUDED_toolkit_intrusivelist_hh
#define INCLUDED_toolkit_intrusivelist_hh

////////////////////////////////////////////////////////////////////////////////
namespace lptk
{

class IntrusiveNode;

struct IntrusiveListHead {
	IntrusiveListHead() : m_head(nullptr), m_tail(nullptr), m_size(0) {}
	IntrusiveNode* m_head;
	IntrusiveNode* m_tail;
	size_t m_size;

	void PushFront(IntrusiveNode *node);
	void PushBack(IntrusiveNode *node);
	void Unlink(IntrusiveNode* node);
} ;

template<class T, IntrusiveNode T::* NodeMember>
class IntrusiveList
{
public:
	IntrusiveList() {}
	IntrusiveList(const IntrusiveList& o) { unused_arg(o); } // no-op copy
	IntrusiveList& operator=(const IntrusiveList& o) { 
		if(this != &o) {
			m_list.m_head = m_list.m_tail = nullptr;
			m_list.m_size = 0;
		}
		return *this;
	}
	~IntrusiveList(); 
	
	class iterator;
	class const_iterator;

	void push_front(T* ptr);
	void push_back(T* ptr);
	void remove(T* ptr);
	void remove(iterator it);
	T* front() const;
	T* back() const;

	iterator begin();
	iterator end();

	const_iterator begin() const;
	const_iterator end() const;
	
	static T* GetObj(IntrusiveNode* ptr) { 
		unsigned int offset = ((size_t) &( ((T*)0)->*NodeMember) );
		return reinterpret_cast<T*>( reinterpret_cast<char*>(ptr) - offset );
	}
	static IntrusiveNode* GetNode(T* ptr) { return &(ptr->*NodeMember); }

	size_t size() const { return m_list.m_size; }
	bool empty() const { return m_list.m_size == 0; }
	void clear() ;

	T* prev(T* cur) const;
	T* next(T* cur) const;
private:
	IntrusiveListHead m_list;
};

class IntrusiveNode
{
	template< class T, IntrusiveNode T::* NodeMember>
	friend class IntrusiveList;
	friend struct IntrusiveListHead;
public:
	IntrusiveNode() : m_list(nullptr), m_next(nullptr), m_prev(nullptr) {}
	IntrusiveNode(const IntrusiveNode&) : m_list(nullptr), m_next(nullptr), m_prev(nullptr) {/* do nothing */}
	IntrusiveNode& operator=(const IntrusiveNode& o) { 
		if(this != &o) { m_list = nullptr; m_next = m_prev = nullptr; } 
		return *this; 
	} 
	// TODO
	IntrusiveNode(IntrusiveNode&& other) : m_list(nullptr), m_next(nullptr), m_prev(nullptr) {
		if(other.m_list)
			MoveNode(this, &other);
	}

	IntrusiveNode& operator=(IntrusiveNode&& other) {
		if(other.m_list)
			MoveNode(this, &other);
		else
			this->Unlink();
		return *this;
	}

	~IntrusiveNode() { Unlink(); }

	void Unlink();
	bool IsLinked() const { return m_list != nullptr; }
private:
	static void MoveNode(IntrusiveNode* dest, IntrusiveNode* src);

	IntrusiveListHead* m_list;
	IntrusiveNode* m_next;
	IntrusiveNode* m_prev;
};
	
inline void IntrusiveNode::MoveNode(IntrusiveNode* dest, IntrusiveNode* src)
{
	ASSERT(dest && src);
	dest->Unlink();

	IntrusiveListHead *list = src->m_list;
	dest->m_list = src->m_list;
	dest->m_prev = src->m_prev;
	dest->m_next = src->m_next;
	src->m_list = nullptr;
	src->m_prev = nullptr;
	src->m_next = nullptr;

	// patch pointers
	if(dest->m_prev) dest->m_prev->m_next = dest;
	if(dest->m_next) dest->m_next->m_prev = dest;

	if(list->m_head == src)
		list->m_head = dest;
	if(list->m_tail == src)
		list->m_tail = dest;
}

inline void IntrusiveNode::Unlink()
{
	if(m_list) {
		m_list->Unlink(this);
	}
}

////////////////////////////////////////////////////////////////////////////////
inline void IntrusiveListHead::Unlink(IntrusiveNode* node)
{
	if(m_head == node)
		m_head = node->m_next;
	if(m_tail == node)
		m_tail = node->m_prev;

	if(node->m_next) 
		node->m_next->m_prev = node->m_prev;
	if(node->m_prev) 
		node->m_prev->m_next = node->m_next;
	node->m_next = nullptr;
	node->m_prev = nullptr;
	node->m_list = nullptr;
		
	--m_size;
}
	
inline void IntrusiveListHead::PushFront(IntrusiveNode *node)
{
	ASSERT(!node->IsLinked());
	if(m_head) {
		node->m_next = m_head;
		m_head->m_prev = node;
		m_head = node;
	} else {
		m_head = m_tail = node;
	}
	node->m_list = this;
	++m_size;
}

inline void IntrusiveListHead::PushBack(IntrusiveNode *node)
{
	ASSERT(!node->IsLinked());
	if(m_tail) {
		node->m_prev = m_tail;
		m_tail->m_next = node;
		m_tail = node;
	} else {
		m_head = m_tail = node;
	}
	node->m_list = this;
	++m_size;
}
	
////////////////////////////////////////////////////////////////////////////////
template<class T, IntrusiveNode T::* NodeMember>
class IntrusiveList<T, NodeMember>::iterator
{
	friend class IntrusiveList<T, NodeMember>;
public:
	iterator() : m_node(nullptr) {}
	iterator(const iterator& o) : m_node(o.m_node) {}
	iterator& operator=(const iterator& o) {
		if(&o != this) {
			m_node = o.m_node;
		}
		return *this;
	}

	bool operator==(const iterator& o) const {
		return m_node == o.m_node;
	}
	bool operator!=(const iterator& o) const {
		return m_node != o.m_node;
	}

	T* operator->() const { return IntrusiveList<T, NodeMember>::GetObj(m_node); }
	T& operator*() const { return *IntrusiveList<T, NodeMember>::GetObj(m_node); }

	iterator& operator++() {
		m_node = m_node->m_next;
		return *this;
	}
	iterator operator++(int) {
		iterator result = *this;
		m_node = m_node->m_next;
		return result;
	}

	iterator& operator--() {
		m_node = m_node->m_prev;
		return *this;
	}

	iterator operator--(int) {
		iterator result = *this;
		m_node = m_node->m_prev;
		return result;
	}
	
private:
	iterator(IntrusiveNode* node) : m_node(node) {}
	IntrusiveNode *m_node;
};

template<class T, IntrusiveNode T::* NodeMember>
class IntrusiveList<T, NodeMember>::const_iterator
{
	friend class IntrusiveList<T, NodeMember>;
public:
	const_iterator() : m_node(nullptr) {}
	const_iterator(const const_iterator& o) : m_node(o.m_node) {}
	const_iterator& operator=(const const_iterator& o) {
		if(&o != this) {
			m_node = o.m_node;
		}
		return *this;
	}

	bool operator==(const const_iterator& o) const {
		return m_node == o.m_node;
	}
	bool operator!=(const const_iterator& o) const {
		return m_node != o.m_node;
	}

	const T* operator->() const { return IntrusiveList<T, NodeMember>::GetObj(m_node); }
	const T& operator*() const { return *IntrusiveList<T, NodeMember>::GetObj(m_node); }

	const_iterator& operator++() {
		m_node = m_node->m_next;
		return *this;
	}
	const_iterator operator++(int) {
		const_iterator result = *this;
		m_node = m_node->m_next;
		return result;
	}

	const_iterator& operator--() {
		m_node = m_node->m_prev;
		return *this;
	}

	const_iterator operator--(int) {
		const_iterator result = *this;
		m_node = m_node->m_prev;
		return result;
	}
	
private:
	const_iterator(IntrusiveNode* node) : m_node(node) {}
	IntrusiveNode *m_node;
};

////////////////////////////////////////////////////////////////////////////////
template<class T, IntrusiveNode T::* NodeMember>
IntrusiveList<T,NodeMember>::~IntrusiveList() {
	IntrusiveNode* cur = m_list.m_head;
	while(cur) {
		IntrusiveNode* next = cur->m_next;
		cur->Unlink();
		cur = next;
	}
}
	
template<class T, IntrusiveNode T::* NodeMember>
typename IntrusiveList<T, NodeMember>::iterator IntrusiveList<T, NodeMember>::begin()
{
	return iterator(m_list.m_head);
}

template<class T, IntrusiveNode T::* NodeMember>
typename IntrusiveList<T, NodeMember>::iterator IntrusiveList<T, NodeMember>::end()
{
	return iterator(nullptr);
}

template<class T, IntrusiveNode T::* NodeMember>
typename IntrusiveList<T, NodeMember>::const_iterator IntrusiveList<T, NodeMember>::begin() const
{
	return const_iterator(m_list.m_head);
}

template<class T, IntrusiveNode T::* NodeMember>
typename IntrusiveList<T, NodeMember>::const_iterator IntrusiveList<T, NodeMember>::end() const
{
	return const_iterator(nullptr);
}

template<class T, IntrusiveNode T::* NodeMember>
void IntrusiveList<T,NodeMember>::push_front(T* ptr)
{
	IntrusiveNode* ptrNode = GetNode(ptr);
	ASSERT(!ptrNode->IsLinked());
	m_list.PushFront(ptrNode);

}

template<class T, IntrusiveNode T::* NodeMember>
void IntrusiveList<T,NodeMember>::push_back(T* ptr)
{
	IntrusiveNode* ptrNode = GetNode(ptr);
	ASSERT(!ptrNode->IsLinked());
	m_list.PushBack(ptrNode);
}

template<class T, IntrusiveNode T::* NodeMember>
void IntrusiveList<T,NodeMember>::remove(T* ptr)
{
	IntrusiveNode* ptrNode = GetNode(ptr);
	ASSERT(ptrNode->IsLinked());
	ASSERT(m_list.m_size > 0);
	ptrNode->Unlink();
}
	
template<class T, IntrusiveNode T::* NodeMember>
T* IntrusiveList<T,NodeMember>::front() const
{
	return m_list.m_head ? GetObj(m_list.m_head) : nullptr;
}

template<class T, IntrusiveNode T::* NodeMember>
T* IntrusiveList<T,NodeMember>::back() const
{
	return m_list.m_tail ? GetObj(m_list.m_tail) : nullptr;
}
	
template<class T, IntrusiveNode T::* NodeMember>
T* IntrusiveList<T,NodeMember>::prev(T* cur) const
{
	if(cur)
	{
		IntrusiveNode* ptrNode = GetNode(cur);
		return ptrNode->m_prev != nullptr ? GetObj(ptrNode->m_prev) : nullptr;
	}
	return nullptr;
}

template<class T, IntrusiveNode T::* NodeMember>
T* IntrusiveList<T,NodeMember>::next(T* cur) const
{
	if(cur)
	{
		IntrusiveNode* ptrNode = GetNode(cur);
		return ptrNode->m_next != nullptr ? GetObj(ptrNode->m_next) : nullptr;
	}
	return nullptr;
}

template<class T, IntrusiveNode T::* NodeMember>
void IntrusiveList<T,NodeMember>::clear() 
{
	IntrusiveNode* cur = m_list.m_head;
	while(cur) {
		IntrusiveNode* next = cur->m_next;
		cur->Unlink();
		cur = next;
	}
	ASSERT(m_list.m_head == nullptr && m_list.m_tail == nullptr &&
		m_list.m_size == 0);
}

////////////////////////////////////////////////////////////////////////////////
// depends on def of iterator
template<class T, IntrusiveNode T::* NodeMember>
void IntrusiveList<T,NodeMember>::remove(iterator it)
{
	remove(&(*it));
}

}

#endif

