#pragma once
#ifndef INCLUDED_TOOLKIT_INTRUSIVELIST_HH
#define INCLUDED_TOOLKIT_INTRUSIVELIST_HH

////////////////////////////////////////////////////////////////////////////////
namespace lptk
{
    template<typename T>
    struct IntrusiveListDLNodeTraits {
        using NodeTraits = typename T::NodeTraits;
        static T* GetNext(T* ptr) { return NodeTraits::GetNext(ptr); }
        static void SetNext(T* ptr, T* next) { NodeTraits::SetNext(ptr, next); }
        static T* GetPrev(T* ptr) { return NodeTraits::GetPrev(ptr); }
        static void SetPrev(T* ptr, T* prev) { NodeTraits::SetPrev(ptr, prev); }
        static void Unlink(T* ptr) {
            auto prev = GetPrev(ptr);
            auto next = GetNext(ptr);
            if (prev)
                SetNext(prev, next);
            if (next)
                SetPrev(next, prev);
        }
    };

    // double-linked intrusive list
    template<typename T, typename NodeTraits = IntrusiveListDLNodeTraits<T> >
    class IntrusiveListDL
    {
        using NodeType = T;
        NodeType* m_head = nullptr;
        NodeType* m_tail = nullptr;
        size_t m_size = 0;
    public:
        IntrusiveListDL() = default;
        IntrusiveListDL(const IntrusiveListDL&) = delete;
        IntrusiveListDL& operator=(const IntrusiveListDL&) = delete;
        IntrusiveListDL(IntrusiveListDL&& o);
        IntrusiveListDL& operator=(IntrusiveListDL&& o);
    
        class iterator;
        class const_iterator;
    
        iterator begin() { return iterator{ m_head }; }
        iterator end() { return iterator{ nullptr }; }

        const_iterator begin() const { return const_iterator(m_head); }
        const_iterator end() const { return const_iterator(nullptr); }

        void push_front(NodeType* node);
        void push_back(NodeType* node);
        NodeType* pop_front();
        NodeType* pop_back();
        iterator erase(NodeType* node);
        void clear();

        size_t size() const { return m_size; }
        bool empty() const { return m_size != 0; }

        NodeType* front() { return m_head; }
        const NodeType* front() const { return m_head; }

        NodeType* back() { return m_tail; }
        const NodeType* back() const { return m_tail; }
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T, typename NT>
    class IntrusiveListDL<T, NT>::iterator
    {
        using NodeType = T;
        NodeType* m_cur = nullptr;
    public:
        iterator() = default;
        iterator(NodeType* cur) : m_cur(cur) {}
        iterator(const iterator& o) = default;
        iterator& operator=(const iterator& o) = default;
        iterator(iterator&& o) = default;
        iterator& operator=(iterator&& o) = default;

        bool operator==(const iterator& o) const { return m_cur == o.m_cur; }
        bool operator!=(const iterator& o) const { return m_cur != o.m_cur; }

        T* operator->() const { return m_cur; }
        T& operator*() const { return *m_cur; }

        iterator& operator++() { 
            m_cur = NT::GetNext(m_cur);
            return *this;
        }
        iterator operator++(int) {
            iterator result = *this;
            m_cur = NT::GetNext(m_cur);
            return result;
        }
    };
    
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T, typename NT>
    class IntrusiveListDL<T, NT>::const_iterator
    {
        using NodeType = T;
        const NodeType* m_cur = nullptr;
    public:
        const_iterator() = default;
        const_iterator(const NodeType* cur) : m_cur(cur) {}
        const_iterator(const const_iterator& o) = default;
        const_iterator& operator=(const const_iterator& o) = default;
        const_iterator(const_iterator&& o) = default;
        const_iterator& operator=(const_iterator&& o) = default;

        bool operator==(const const_iterator& o) const { return m_cur == o.m_cur; }
        bool operator!=(const const_iterator& o) const { return m_cur != o.m_cur; }

        const T* operator->() const { return m_cur; }
        const T& operator*() const { return *m_cur; }

        const_iterator& operator++() { 
            m_cur = NT::GetNext(m_cur);
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator result = *this;
            m_cur = NT::GetNext(m_cur);
            return result;
        }
    };

    
    template<typename T, typename NT>
    IntrusiveListDL<T, NT>::IntrusiveListDL(IntrusiveListDL&& o)
    {
        m_head = std::exchange(o.m_head, nullptr);
        m_tail = std::exchange(o.m_tail, nullptr);
        m_size = std::exchange(o.m_size, 0);
    }

    template<typename T, typename NT>
    auto IntrusiveListDL<T, NT>::operator=(IntrusiveListDL&& o) -> IntrusiveListDL&
    {
        m_head = std::exchange(o.m_head, nullptr);
        m_tail = std::exchange(o.m_tail, nullptr);
        m_size = std::exchange(o.m_size, 0);
        return *this;
    }
        
    template<typename T, typename NT>
    auto IntrusiveListDL<T, NT>::push_front(NodeType* node) -> void
    {
        if (m_head)
        {
            NT::SetNext(node, m_head);
            NT::SetPrev(m_head, node);
            m_head = node;
        }
        else
        {
            m_head = m_tail = node;
        }
        ++m_size;
    }

    template<typename T, typename NT>
    auto IntrusiveListDL<T, NT>::push_back(NodeType* node) -> void
    {
        if (m_tail)
        {
            NT::SetNext(m_tail, node);
            NT::SetPrev(node, m_tail);
            m_tail = node;
        }
        else
        {
            m_head = m_tail = node;
        }
        ++m_size;
    }

    template<typename T, typename NT>
    auto IntrusiveListDL<T, NT>::pop_front() -> NodeType*
    {
        NodeType* result = nullptr;
        if (m_head)
        {
            result = m_head;
            m_head = NT::GetNext(m_head);
            if (!m_head)
                m_tail = nullptr;
            else
                NT::SetPrev(m_head, nullptr);
            NT::SetNext(result, nullptr);
            --m_size;
        }
        return result;
    }

    template<typename T, typename NT>
    auto IntrusiveListDL<T, NT>::pop_back() -> NodeType*
    {
        NodeType* result = nullptr;
        if (m_tail)
        {
            result = m_tail;
            m_tail = NT::GetPrev(m_tail);
            if (!m_tail)
                m_head = nullptr;
            else
                NT::SetNext(m_tail, nullptr);
            NT::SetPrev(result, nullptr);
            --m_size;
        }
        return result;
    }
        
    template<typename T, typename NT>
    auto IntrusiveListDL<T, NT>::erase(NodeType* node) -> iterator
    {
        if (node)
        {
            auto nextPtr = NT::GetNext(node);
            NT::Unlink(node);
            --m_size;
            return iterator(nextPtr);
        }
        return nullptr;
    }
        
    template<typename T, typename NT>
    auto IntrusiveListDL<T, NT>::clear() -> void
    {
        auto cur = m_head;
        while (cur)
        {
            auto next = NT::GetNext(cur);
            NT::Unlink(cur);
            cur = next;
        }
        m_head = m_tail = nullptr;
    }

}

#endif

