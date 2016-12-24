#pragma once
#ifndef INCLUDED_TOOLKIT_CIRCULARQUEUE_HH
#define INCLUDED_TOOLKIT_CIRCULARQUEUE_HH

#include <memory>

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    class CircularQueue
    {
    public:
        explicit CircularQueue(unsigned size);

        CircularQueue(CircularQueue&&) = default;
        CircularQueue& operator=(CircularQueue&&) = default;

        bool push(T val);
        bool pop(T& val);

        bool empty() const;
        bool full() const;

    private:
        std::unique_ptr<T[]> m_buffer;
        unsigned m_size = 0;
        unsigned m_readPos = 0;
        unsigned m_writePos = 0;
    };


    ////////////////////////////////////////////////////////////////////////////////
    template<typename T>
    CircularQueue<T>::CircularQueue(unsigned size)
        : m_buffer(new T[size])
        , m_size(size)
    {
    }
        
    template<typename T>
    bool CircularQueue<T>::push(T val)
    {
        const auto writePos = m_writePos;
        const auto readPos = m_readPos;
        const auto size = m_size;
        if (readPos != writePos && (readPos % size) == (writePos % size))
            return false;

        m_buffer[writePos % size] = val;
        
        const auto newWritePos = (writePos + 1) % (size << 1);
        m_writePos = newWritePos;
        return true;
    }

    template<typename T>
    bool CircularQueue<T>::pop(T& val)
    {
        const auto writePos = m_writePos;
        const auto readPos = m_readPos;

        if (readPos == writePos)
            return false;

        const auto size = m_size;
        val = m_buffer[readPos % size];
        
        const auto newReadPos = (readPos + 1) % (size << 1);
        m_readPos = newReadPos;
        return true;
    }

    template<typename T>
    bool CircularQueue<T>::empty() const
    {
        return m_readPos == m_writePos;
    }
    
    template<typename T>
    bool CircularQueue<T>::full() const
    {
        return m_readPos != m_writePos && 
            (m_readPos % m_size) == (m_writePos % m_size);
    }
}

#endif

