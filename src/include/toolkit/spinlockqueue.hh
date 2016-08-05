#pragma once
#ifndef INCLUDED_toolkit_spinlockqueue_HH
#define INCLUDED_toolkit_spinlockqueue_HH

#include <atomic>

namespace lptk
{
    template<class T>
    class SpinLockQueue
    {
        enum { kCacheLine = 64 };

        struct Node
        {
            Node(T&& data) : m_data(std::move(data)), m_next(nullptr) {}
            Node(const T& data) : m_data(data), m_next(nullptr) {}
            T m_data;
            std::atomic<Node*> m_next;

            enum { memberSize = sizeof(decltype(m_data)) + sizeof(decltype(m_next)) };
            char padding[((memberSize + kCacheLine - 1) & ~(kCacheLine - 1)) - memberSize];
        };

        Node* m_head;
        char padding0[kCacheLine - sizeof(decltype(m_head))];
        Node* m_tail;
        char padding1[kCacheLine - sizeof(decltype(m_tail))];
        std::atomic<bool> m_headLock;
        char padding2[kCacheLine - sizeof(decltype(m_headLock))];
        std::atomic<bool> m_tailLock;
        char padding3[kCacheLine - sizeof(decltype(m_tailLock))];
    public:
        SpinLockQueue()
            : m_head(nullptr)
            , m_tail(nullptr)
            , m_headLock(false)
            , m_tailLock(false)
        {
            m_head = m_tail = new Node(T());
        }

        ~SpinLockQueue()
        {
            Node* cur = m_head;
            while (cur)
            {
                Node* next = cur->m_next;
                delete cur;
                cur = next;
            }
        }

        template<class U>
        void push(U&& val)
        {
            Node* node = new Node(std::forward<U>(val));

            while (m_tailLock.exchange(true, std::memory_order::memory_order_acq_rel)) {}

            Node* tail = m_tail;
            tail->m_next = node;
            m_tail = node;

            m_tailLock.store(false, std::memory_order::memory_order_release);
        }

        bool pop(T& result)
        {
            while (m_headLock.exchange(true, std::memory_order::memory_order_acq_rel)) {}

            Node* first = m_head;
            Node* next = first->m_next;

            if (next != nullptr)
            {
                result = std::move(next->m_data);
                m_head = next;

                m_headLock.store(false, std::memory_order::memory_order_release);

                delete first;
                return true;
            }
            else
            {
                m_headLock.store(false, std::memory_order::memory_order_release);
            }

            return false;
        }
        
        T pop()
        {
            while (m_headLock.exchange(true, std::memory_order::memory_order_acq_rel)) {}

            Node* first = m_head;
            Node* next = first->m_next;
            auto result = T();

            if (next != nullptr)
            {
                result = std::move(next->m_data);
                m_head = next;

                m_headLock.store(false, std::memory_order::memory_order_release);

                delete first;
            }
            else
            {
                m_headLock.store(false, std::memory_order::memory_order_release);
            }

            return result;
        }
    };
}

#endif
