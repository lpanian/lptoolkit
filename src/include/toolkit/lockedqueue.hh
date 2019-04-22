#pragma once

#include <mutex>

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    template<class T>
    class LockedQueue
    {
        struct Node
        {
            Node(const T& data) : m_data(data), m_next(nullptr) {}
            Node(T&& data) : m_data(std::move(data)), m_next(nullptr) {}
            T m_data;
            Node* m_next;
        };

        Node* m_head;
        Node* m_tail;
        std::mutex m_mutex;
        std::mutex m_tailmutex;

    public:
        LockedQueue()
            : m_head(nullptr)
            , m_tail(nullptr)
        {
            m_head = m_tail = new Node(T());
        }

        ~LockedQueue()
        {
            Node* cur = m_head;
            while (cur)
            {
                Node* next = cur->m_next;
                delete cur;
                cur = next;
            }
        }

        void push(const T& val)
        {
            Node* node = new Node(val);

            std::lock_guard<std::mutex> lock(m_tailmutex);
            m_tail->m_next = node;
            m_tail = node;
        }
        
        void push(T&& val)
        {
            Node* node = new Node(std::move(val));

            std::lock_guard<std::mutex> lock(m_tailmutex);
            m_tail->m_next = node;
            m_tail = node;
        }

        bool pop(T& result)
        {
            std::unique_lock<std::mutex> lock(m_mutex);

            Node* first = m_head;
            Node* next = first->m_next;

            if (next != nullptr)
            {
                // do this first, if copy throws then we live!
                result = std::move(next->m_data);
                m_head = next;

                lock.unlock();

                delete first;
                return true;
            }

            return false;
        }
    };
    
    
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    struct IntrusiveLockQueueNodeTraits {
        using NodeTraits = typename T::NodeTraits;
        static T* GetNext(T* ptr) { return NodeTraits::GetNext(ptr); }
        static void SetNext(T* ptr, T* next) { NodeTraits::SetNext(ptr, next); }
    };

    template<typename T, typename NodeTraits = IntrusiveLockQueueNodeTraits<T> >
    class IntrusiveLockedQueue
    {
        using NodeType = T;
        NodeType* m_head = nullptr;
        NodeType* m_tail = nullptr;
        std::mutex m_mutex;
    public:
        void push(NodeType* node);
        NodeType* pop();
    };
   
    template<typename T, typename NT>
    auto IntrusiveLockedQueue<T, NT>::push(NodeType* node) -> void
    {
        NT::SetNext(node, nullptr);

        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_tail)
        {
            NT::SetNext(m_tail, node);
            m_tail = node;
        }
        else
        {
            m_head = m_tail = node;
        }
    }

    template<typename T, typename NT>
    auto IntrusiveLockedQueue<T, NT>::pop() -> NodeType*
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto result = m_head;
        if (result)
        {
            m_head = NT::GetNext(m_head);
            if (!m_head)
                m_tail = nullptr;
        }
        lock.unlock();

        if (result)
            NT::SetNext(result, nullptr);
        return result;
    }
}
