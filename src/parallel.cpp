#include "toolkit/parallel.hh"

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    void Semaphore::Acquire()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return m_count > 0; });
        --m_count;
    }

    void Semaphore::Release()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto old = m_count++;
        lock.unlock();

        if (old == 0)
            m_cv.notify_all();
    }
}
