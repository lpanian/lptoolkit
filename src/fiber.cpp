#include "toolkit/fiber.hh"

#include "toolkit/thread.hh"
#include "toolkit/dynary.hh"

namespace lptk
{
    namespace fiber
    {
        ////////////////////////////////////////////////////////////////////////////////
        class FiberManager
        {
        public:
            FiberManager();

            FiberManager(const FiberManager&) = delete;
            FiberManager& operator=(const FiberManager&) = delete;
            FiberManager(FiberManager&&) = delete;
            FiberManager& operator=(FiberManager&&) = delete;
            
            ~FiberManager();

            static void Reserve(int numThreads);
            static FiberManager* Get();

            static void InitThread();

            void ScheduleFiber();
            void CompleteFiber();
            void Push(Fiber* fiber);
            Fiber* Pop();
            Fiber* Steal();
            void YieldFiber();

        private:
            Fiber* StealPop();
            std::atomic<Fiber*> m_fiberWaiting = nullptr;
            std::atomic<Fiber*> m_fiberQueue = nullptr;
            void* m_mainFiber = nullptr;
            Fiber* m_curFiber = nullptr;
            int m_stealIndex = 0;
            void* m_fiberToDestroy = nullptr;

            static std::atomic<int> s_nextIndex;
            static lptk::DynAry<std::unique_ptr<FiberManager>> s_managers;
            static thread_local int s_managerIndex;
        };

        thread_local int FiberManager::s_managerIndex = -1;
        std::atomic<int> FiberManager::s_nextIndex = 0;
        lptk::DynAry<std::unique_ptr<FiberManager>> FiberManager::s_managers;
            
        void FiberManager::Reserve(int numThreads)
        {
            s_managers.reserve(numThreads);
            while (numThreads-- > 0)
                s_managers.push_back(make_unique<FiberManager>());
        }
            
        FiberManager* FiberManager::Get()
        {
            ASSERT(unsigned(s_managerIndex) < s_managers.size());
            if (s_managerIndex < 0 || unsigned(s_managerIndex) > s_managers.size())
                return nullptr;
            return s_managers[s_managerIndex].get();
        }

        FiberManager::FiberManager()
        {

        }
            
        FiberManager::~FiberManager()
        {

        }
            
        void FiberManager::InitThread()
        {
            s_managerIndex = s_nextIndex++;

#if defined(WINDOWS)
            Get()->m_mainFiber = ConvertThreadToFiber(nullptr);
#elif defined(LINUX)
#error linux version of FiberManager::InitThread not yet implemented
#endif
        }
            
        void FiberManager::ScheduleFiber()
        {
            ASSERT(this == Get());
            Fiber* fiber = nullptr;

#if defined(WINDOWS)
            // If we're in the 'scheduler' thread, select a new fiber before
            // pushing our old one.
            if (GetCurrentFiber() == m_mainFiber)
            {
                if (m_fiberToDestroy)
                {
                    DeleteFiber(m_fiberToDestroy);
                    m_fiberToDestroy = nullptr;
                }

                fiber = Pop();
                if (!fiber)
                    fiber = Steal();
                
                if (m_curFiber)
                {
                    Push(m_curFiber);
                    m_curFiber = nullptr;
                }
               
                if (fiber)
                {
                    ASSERT(fiber->m_next == nullptr);

                    m_curFiber = fiber;
                    ASSERT(fiber->m_fiber != GetCurrentFiber());
                    SwitchToFiber(fiber->m_fiber);
                }
            }
            else
            {
                SwitchToFiber(m_mainFiber);
            }
#elif defined(LINUX)
#error linux version of FiberManager::ScheduleFiber not yet implemented
#endif
        }
            
        void FiberManager::CompleteFiber()
        {
            ASSERT(this == Get());
            ASSERT(m_fiberToDestroy == nullptr);

            m_fiberToDestroy = m_curFiber->m_fiber;
            m_curFiber->m_fiber = nullptr;
            m_curFiber = nullptr;
        }

        void FiberManager::Push(Fiber* fiber)
        {
            fiber->m_next = m_fiberWaiting.load(std::memory_order_acquire);
            while (!m_fiberWaiting.compare_exchange_weak(fiber->m_next, fiber, std::memory_order_acq_rel)) {}
        }

        Fiber* FiberManager::Pop()
        {
            // see if there is another fiber ready in the queue
            auto fiber = m_fiberQueue.load(std::memory_order_acquire);
            while (!m_fiberQueue.compare_exchange_weak(fiber, fiber ? fiber->m_next : nullptr, std::memory_order_acq_rel)) {}

            if (!fiber)
            {
                // we didn't get a fiber in the ready queue, so enqueue all waiting fibers
                auto waitingTop = m_fiberWaiting.exchange(nullptr, std::memory_order_acq_rel);
                while (waitingTop)
                {
                    fiber = waitingTop;
                    waitingTop = waitingTop->m_next;

                    // only enqueue if we're not the very last waiting one (meaning the first that was queued), 
                    // because that fiber would be the first result we want to run.
                    if (fiber->m_next)
                    {
                        fiber->m_next = m_fiberQueue.load(std::memory_order_acquire);
                        while (!m_fiberQueue.compare_exchange_weak(fiber->m_next, fiber, std::memory_order_acq_rel)) {}
                    }
                }
            }

            if (fiber)
            {
                fiber->m_next = nullptr;
            }
            return fiber;
        }
            
        Fiber* FiberManager::StealPop()
        {
            auto result = static_cast<Fiber*>(nullptr);
            // try the waiting fibers first to avoid touching 'working' fibers
            auto waitingTop = m_fiberWaiting.exchange(nullptr, std::memory_order_acq_rel);
            while (waitingTop)
            {
                result = waitingTop;
                waitingTop = waitingTop->m_next;

                // push all but the very last one onto the working queue
                if (result->m_next)
                {
                    result->m_next = m_fiberQueue.load(std::memory_order_acquire);
                    while (!m_fiberQueue.compare_exchange_weak(result->m_next, result, std::memory_order_acq_rel)) {}
                }
            }

            if (!result)
            {
                // nothing to steal in the waiting queue, so pop from the active queue
                result = m_fiberQueue.load(std::memory_order_acquire);
                while (!m_fiberQueue.compare_exchange_weak(result, result ? result->m_next : nullptr, std::memory_order_acq_rel)) {}
            }
                
            if (result)
            {
                result->m_next = nullptr;
            }
            return result;
        }
            
        Fiber* FiberManager::Steal()
        {
            if (s_managerIndex < 0)
                return nullptr;

            const auto numThreads = int(s_managers.size());
            auto index = m_stealIndex % numThreads;
            if (index == s_managerIndex)
            {
                index = (index + 1) % numThreads;
            }

            if (index != s_managerIndex)
            {
                auto mgr = s_managers[index].get();
                m_stealIndex = (index + 1) % numThreads;

                if (!mgr)
                    return nullptr;

                auto fiber = mgr->StealPop();
                if (fiber)
                {
                    return fiber;
                }
            }
            
            return nullptr;
        }
            
        void FiberManager::YieldFiber()
        {
            ScheduleFiber();
        }



        ////////////////////////////////////////////////////////////////////////////////
        Fiber::Fiber(Counter* counter, FiberFunc fn, void* userData)
            : m_counter(counter)
            , m_fn(fn)
            , m_userData(userData)
        {
            m_counter->IncRef();
#if defined(WINDOWS)
            m_fiber = CreateFiber(64 * 1024, &FiberMain, this);
#endif
            
            FiberManager::Get()->Push(this);
        }

        Fiber::~Fiber()
        {
            ASSERT(m_counter->IsZero());
        }
        
        void Fiber::Run()
        {
            m_fn(m_userData);

            FiberManager::Get()->CompleteFiber();
            m_counter->DecRef();
            FiberManager::Get()->ScheduleFiber();
        }
       
#if defined(WINDOWS)
        void CALLBACK Fiber::FiberMain(void* param)
        {
            reinterpret_cast<Fiber*>(param)->Run();
        }
#endif


        ////////////////////////////////////////////////////////////////////////////////     
        static std::atomic<int> g_finished;
        static void FiberWorkerThread()
        {
            FiberManager::InitThread();
            while (!g_finished.load(std::memory_order_relaxed))
            {
                FiberManager::Get()->ScheduleFiber();
            }
        }

        static lptk::DynAry<lptk::Thread> g_workerThreads;
        bool Init(int numWorkers)
        {
            FiberManager::Reserve(numWorkers + 1);
            FiberManager::InitThread();
            g_workerThreads.reserve(numWorkers);
            for (int i = 0; i < numWorkers; ++i)
                g_workerThreads.push_back(lptk::Thread(FiberWorkerThread));
            return true;
        }

        bool Purge()
        {
            g_finished.store(std::memory_order_release);
            for (auto&& th : g_workerThreads)
            {
                th.join();
            }
            return true;
        }
        
        void WaitForCounter(Counter* counter)
        {
            while (!counter->IsZero())
            {
                FiberManager::Get()->YieldFiber();
            }
        }

        void YieldFiber()
        {
            FiberManager::Get()->YieldFiber();
        }
    }
}