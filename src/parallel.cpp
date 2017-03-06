#include "toolkit/parallel.hh"

namespace lptk
{
    ////////////////////////////////////////////////////////////////////////////////
    void Semaphore::Acquire(unsigned long val)
    {
        while (val != 0)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this, val] {
                return m_count > 0;
            });
            const auto numToReduce = val < m_count ? val : m_count;
            m_count -= numToReduce;
            lock.unlock();
            val -= numToReduce;
        }
    }

    void Semaphore::Release(unsigned long val)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto old = m_count;
        m_count += val;
        lock.unlock();

        if (old == 0)
            m_cv.notify_all();
    }
}
