#pragma once
#ifndef INCLUDED_toolkit_lockedqueue_HH
#define INCLUDED_toolkit_lockedqueue_HH

#include <mutex>

namespace lptk
{
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
}

#endif
