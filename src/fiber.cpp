#include "toolkit/fiber.hh"

#include <mutex>
#include <condition_variable>

#include "toolkit/thread.hh"
#include "toolkit/dynary.hh"

#include "toolkit/spinlockqueue.hh"
#include "toolkit/circularqueue.hh"
#include "toolkit/parallel.hh"

namespace lptk
{
    namespace fiber
    {
        class FiberPool;

        ////////////////////////////////////////////////////////////////////////////////
        struct Task::NodeTraits
        {
            static Task* GetNext(Task* ptr) { return ptr->m_next; }
            static void SetNext(Task* ptr, Task* next) { ptr->m_next = next; }
        };

        void Task::Execute()
        {
            (*m_task)(m_userData);
            m_counter->DecRef();
        }


        ////////////////////////////////////////////////////////////////////////////////
        class Fiber
        {
            friend class FiberManager;
            friend struct FiberNodeTraits;
        public:
            using NodeTraits = FiberNodeTraits;
            using FiberHandle = void*;

            Fiber();

            Fiber(const Fiber&) = delete;
            Fiber& operator=(const Fiber&) = delete;
            Fiber(Fiber&& other) = delete;
            Fiber& operator=(Fiber&& other) = delete;

            ~Fiber();

            bool Init(unsigned stackSize);
            bool InitWithOwner(int ownerIndex, FiberHandle existingHandle);
            void Release();

            void Run();
            void Continue();
        private:

            int m_ownerThread = -1;
            Fiber* m_next = nullptr;

#if defined(WINDOWS)
            static void CALLBACK FiberMain(void* param);
            // actual implementation pointer (void* on windows)
            void* m_fiber = nullptr;
#elif defined(LINUX)
#error LinuxFiber implementation not yet implemented. You should do that!
#endif
        };

        Fiber* FiberNodeTraits::GetNext(Fiber* ptr)
        {
            return ptr->m_next;
        }

        void FiberNodeTraits::SetNext(Fiber* ptr, Fiber* next)
        {
            ptr->m_next = next;
        }

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

            static bool Init(const FiberInitStruct& init);
            static bool Purge();
            static FiberManager* Get();
        
            void RunTasks(Task* tasks, size_t numTasks, Counter* counter, int priority);
            
            void YieldFiber();
            void WaitForCounter(Counter* counter);
            void YieldFiberToService(FiberService* service, void* requestData);
            bool IsExitRequested() const;
          
            bool IsInFiberThread();
            int GetFiberThreadId();

            // used when starting a fiber to initialize the current fiber. 
            // Returns true if another fiber was run, false otherwise.
            void NextFiber();

            // wait until we have a fiber available or a task available.
            void NotifyTaskComplete();
            void NotifyServiceComplete();
            void WaitForFiber();
            void WaitForTasks();

            // set thread state so it knows it is executing the current fiber
            void ResumeThisFiber(Fiber* currentFiber);
            void FinishFiber();

            // get a task, or return null if none are available
            Task* PopTask();
        private:
            void Cleanup();
            bool InitMain(const FiberInitStruct& init);
            void NotifyWorkerThreadsOfTasks(unsigned numTasks);

            static void WorkerMain(FiberManager* fiberMgr, unsigned threadIndex);

            FiberInitStruct m_init;
            std::atomic<bool> m_exitRequested = false;
            lptk::DynAry<std::unique_ptr<Fiber>> m_fibers;

            struct ThreadData
            {
                Fiber m_threadFiber;
                Fiber* m_currentFiber = nullptr;
                Fiber* m_lastFiber = nullptr;
                lptk::IntrusiveSpinLockQueue<Fiber> m_threadQueue;
            };
            
            lptk::DynAry<lptk::Thread> m_workerThreads;
            lptk::DynAry<std::unique_ptr<ThreadData>> m_threadData;

            lptk::IntrusiveSpinLockQueue<Task> m_lowPriorityTaskQueue;
            lptk::IntrusiveSpinLockQueue<Task> m_highPriorityTaskQueue;
            lptk::IntrusiveSpinLockQueue<Fiber> m_executeQueue;

            std::atomic<unsigned> m_numTasks = 0;
            std::condition_variable m_hasTasksCondition;
            std::mutex m_hasTasksMutex;

            std::atomic<unsigned> m_numWaitingServiceFibers = 0;
            std::condition_variable m_allWaitingCondition;
            std::mutex m_allWaitingMutex;
            unsigned m_maxWaitingServiceFibers = 0;

            static std::unique_ptr<FiberManager> s_ptr;
            static thread_local int s_currentThread;
        };

        std::unique_ptr<FiberManager> FiberManager::s_ptr;
        thread_local int FiberManager::s_currentThread = -1;

        bool FiberManager::Init(const FiberInitStruct& init)
        {
            if (s_ptr)
                return false;

            s_ptr = make_unique<FiberManager>();
            const auto result = s_ptr->InitMain(init);
            if (!result)
                s_ptr.reset();
            return result;
        }

        bool FiberManager::Purge()
        {
            // cause a join before the s_ptr.reset()
            if (!s_ptr)
                return false;

            s_ptr->Cleanup();
            s_ptr.reset();
            return true;
        }
            
        FiberManager* FiberManager::Get()
        {
            return s_ptr.get();
        }
            
        void FiberManager::WorkerMain(FiberManager* fiberMgr, unsigned threadIndex)
        {
            s_currentThread = int(threadIndex);
            auto& threadData = *fiberMgr->m_threadData[threadIndex];

            // initial set up
#if defined(WINDOWS)
            void *mainFiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#elif defined(LINUX)
#error linux version of FiberManager::InitThread not yet implemented
#endif

            threadData.m_threadFiber.InitWithOwner(s_currentThread, mainFiber);

            // if we've exited Run, that means an exit was requested and we're shutting this thread down.
            threadData.m_threadFiber.Run();

            ASSERT(threadData.m_currentFiber->m_ownerThread == s_currentThread);

            // prevent a DeleteFiber call and exit normally
            threadData.m_threadFiber.Release();
        }
        
        void FiberManager::NextFiber()
        {
            auto&& threadData = *m_threadData[s_currentThread];
            const auto currentFiber = threadData.m_currentFiber;
            ASSERT(currentFiber != nullptr);

            Fiber* nextFiber = nullptr;
            nextFiber = threadData.m_threadQueue.pop();
            if(!nextFiber)
                nextFiber = m_executeQueue.pop();
            ASSERT(nextFiber != currentFiber);

            if (nextFiber)
            {
                if (nextFiber->m_ownerThread >= 0 &&
                    nextFiber->m_ownerThread != s_currentThread)
                {
                    m_threadData[nextFiber->m_ownerThread]->m_threadQueue.push(nextFiber);
                    nextFiber = m_executeQueue.pop();
                }

                if (nextFiber)
                {
                    threadData.m_lastFiber = currentFiber;
                    nextFiber->Continue();
                    ResumeThisFiber(currentFiber);
                }
            }
        }

        void FiberManager::WaitForFiber()
        {
            auto numWaiting = m_numWaitingServiceFibers.load(std::memory_order_acquire);
            if (numWaiting == m_maxWaitingServiceFibers)
            {
                std::unique_lock<std::mutex> lock(m_allWaitingMutex);
                m_allWaitingCondition.wait(lock, [&] {
                    numWaiting = m_numWaitingServiceFibers.load(std::memory_order_acquire);
                    return (numWaiting < m_maxWaitingServiceFibers);
                });
            }
        }

        void FiberManager::WaitForTasks()
        {
            if (s_currentThread == 0)
                return;

            auto numTasks = m_numTasks.load(std::memory_order_acquire);
            if (numTasks == 0)
            {
                std::unique_lock<std::mutex> lock(m_hasTasksMutex);
                m_hasTasksCondition.wait(lock, [&] {
                    numTasks = m_numTasks.load(std::memory_order_acquire);
                    return numTasks > 0;
                });
            }
        }

        void FiberManager::ResumeThisFiber(Fiber* currentFiber)
        {
            auto&& threadData = *m_threadData[s_currentThread];
            if (threadData.m_lastFiber)
            {
                m_executeQueue.push(std::exchange(threadData.m_lastFiber, nullptr));
            }
            threadData.m_currentFiber = currentFiber;
        }
            
        void FiberManager::FinishFiber()
        {
            auto&& threadData = *m_threadData[s_currentThread];
            while(threadData.m_currentFiber->m_ownerThread < 0 ||
                threadData.m_currentFiber->m_ownerThread != s_currentThread)
            {
                NextFiber();
            }
        }

        void FiberManager::YieldFiber()
        {
            NextFiber();
        }

        void FiberManager::WaitForCounter(Counter* counter)
        {
            while (!counter->IsZero())
            {
                auto task = PopTask();
                if (task)
                {
                    // run a different task! This helps forward progress
                    // when we have subtasks that also wait on other counters,
                    // at the cost of stack space.
                    task->Execute();
                    NotifyTaskComplete();
                }
                else
                {
                    WaitForTasks();
                    WaitForFiber();
                    NextFiber();
                }
            }
        }

        Task* FiberManager::PopTask()
        {
            Task* task = nullptr;

            // try to grab a high priority task
            task = m_highPriorityTaskQueue.pop();
            if (!task)
            {
                // if that didn't work, grab a low priority task
                task = m_lowPriorityTaskQueue.pop();
            }
            return task;
        }
            
        bool FiberManager::IsInFiberThread()
        {
            return s_currentThread >= 0;
        }
            
        int FiberManager::GetFiberThreadId()
        {
            return s_currentThread;
        }
            
        bool FiberManager::InitMain(const FiberInitStruct& init)
        {
            m_init = init;
            const auto numWorkers = lptk::Max(1u, init.numWorkerThreads);
            const auto numFibers = lptk::Max(1u, init.numFibers);

            m_fibers.resize(numFibers);

            m_maxWaitingServiceFibers = numFibers + numWorkers;

            for (unsigned i = 0; i < numFibers; ++i)
            {
                m_fibers[i] = make_unique<Fiber>();
                m_fibers[i]->Init(init.stackSize);
                m_executeQueue.push(m_fibers[i].get());
            }

            m_workerThreads.reserve(numWorkers - 1);
            m_threadData.resize(numWorkers);
            for (unsigned i = 0; i < numWorkers; ++i)
            {
                m_threadData[i] = make_unique<ThreadData>();
                auto& threadData = *m_threadData[i];

                if (i == 0) // main thread gets to be a fiber too!
                {
                    s_currentThread = 0;
#if defined(WINDOWS)
                    void* mainFiber = ConvertThreadToFiberEx(nullptr, FIBER_FLAG_FLOAT_SWITCH);
#elif defined(LINUX)
#error linux version of FiberManager::InitThread not yet implemented
#endif
                    // null owner means we'll never choose this fiber when scheduling a task - we'll
                    // only be able to return to it from waits and yields.
                    threadData.m_threadFiber.InitWithOwner(s_currentThread, mainFiber);
                    threadData.m_currentFiber = &threadData.m_threadFiber;
                }
                else
                {
                    m_workerThreads.push_back(lptk::Thread(WorkerMain, this, i));
                }
            }

            return true;
        }
            
        FiberManager::FiberManager()
        { }
            
        FiberManager::~FiberManager()
        { 
        }

        void FiberManager::Cleanup()
        {
            m_exitRequested = true;
            
            NotifyWorkerThreadsOfTasks(unsigned(m_workerThreads.size()));
            for (auto&& thread : m_workerThreads)
            {
                if (thread.joinable())
                    thread.join();
            }

            m_threadData[0]->m_threadFiber.Release();
            m_fibers.clear();
            m_threadData.clear();

        }
            
        void FiberManager::RunTasks(Task* tasks, size_t numTasks, Counter* counter, int priority)
        {
            if (numTasks == 0)
                return;
            ASSERT(counter != nullptr);
            counter->IncRef(numTasks);

            for (size_t i = 0; i < numTasks; ++i)
                tasks[i].SetCounter(counter);
            (priority == 0 ? m_lowPriorityTaskQueue : m_highPriorityTaskQueue).push_range(tasks, numTasks);

            NotifyWorkerThreadsOfTasks(unsigned(numTasks));
        }

        void FiberManager::NotifyWorkerThreadsOfTasks(unsigned numTasks)
        {
            auto curNumTasks = m_numTasks.load(std::memory_order_acquire);
            if (curNumTasks == 0)
            {
                std::unique_lock<std::mutex> lock(m_hasTasksMutex);
                m_numTasks.fetch_add(numTasks, std::memory_order_acq_rel);
                lock.unlock();
                m_hasTasksCondition.notify_all();
            }
            else
            {
                m_numTasks.fetch_add(numTasks, std::memory_order_acq_rel);
            }
        }
            
        void FiberManager::YieldFiberToService(FiberService* service, void* requestData)
        {
            auto&& threadData = *m_threadData[s_currentThread];
            const auto fiber = threadData.m_currentFiber;

            Counter serviceCounter;
            serviceCounter.IncRef();
            auto request = FiberService::ServiceRequest{fiber, requestData, &serviceCounter};
            service->PushServiceFiber(&request);
            m_numWaitingServiceFibers.fetch_add(1u, std::memory_order_release);
            while (!serviceCounter.IsZero())
            {
                WaitForFiber();
                NextFiber();
            }
        }
            
        void FiberManager::NotifyTaskComplete()
        {
            const auto numTasks = m_numTasks.load(std::memory_order_acquire);
            if (numTasks == 1u)
            {
                std::unique_lock<std::mutex> lock(m_hasTasksMutex);
                m_numTasks.fetch_sub(1u, std::memory_order_acq_rel);
            }
            else
            {
                m_numTasks.fetch_sub(1u, std::memory_order_acq_rel);
            }

            m_hasTasksCondition.notify_all();
        }

        void FiberManager::NotifyServiceComplete()
        {
            const auto numWaiting = m_numWaitingServiceFibers.load(std::memory_order_acquire);
            if (numWaiting == m_maxWaitingServiceFibers)
            {
                std::unique_lock<std::mutex> lock2(m_allWaitingMutex);
                m_numWaitingServiceFibers.fetch_sub(1u, std::memory_order_acq_rel);
            }
            else
            {
                m_numWaitingServiceFibers.fetch_sub(1u, std::memory_order_acq_rel);
            }
            m_allWaitingCondition.notify_all();
        }

        bool FiberManager::IsExitRequested() const
        {
            return m_exitRequested.load(std::memory_order_relaxed);
        }
       


        ////////////////////////////////////////////////////////////////////////////////
        Fiber::Fiber()
        {
        }

        Fiber::~Fiber()
        {
#if defined(WINDOWS)
            if(m_fiber)
                DeleteFiber(m_fiber);
#endif
        }
            
        bool Fiber::Init(unsigned stackSize)
        {
            ASSERT(m_fiber == nullptr);
#if defined(WINDOWS)
            m_fiber = CreateFiberEx(stackSize, 0, FIBER_FLAG_FLOAT_SWITCH, &FiberMain, this);
#endif
            if (m_fiber == nullptr)
                return false;
            return true;
        }

        bool Fiber::InitWithOwner(int ownerIndex, FiberHandle existingHandle)
        {
            ASSERT(m_fiber == nullptr);
            m_ownerThread = ownerIndex;
#if defined(WINDOWS)
            m_fiber = existingHandle;
#endif
            return true;
        }

        void Fiber::Release() 
        {
            m_fiber = nullptr;
        }

#if defined(WINDOWS)
        void CALLBACK Fiber::FiberMain(void* param)
        {
            auto fiber = reinterpret_cast<Fiber*>(param);
            fiber->Run();
            fiber->Release();
        }
#endif
        
        void Fiber::Continue()
        {
            if (m_fiber)
            {
                SwitchToFiber(m_fiber);
            }
        }

        void Fiber::Run()
        {
            FiberManager::Get()->ResumeThisFiber(this);

            while (!FiberManager::Get()->IsExitRequested())
            {
                auto task = FiberManager::Get()->PopTask();

                if (task)
                {
                    task->Execute();
                    FiberManager::Get()->NotifyTaskComplete();
                }
                else
                {
                    // another fiber might be waiting to run, so give it a chance
                    FiberManager::Get()->WaitForTasks(); 
                    FiberManager::Get()->WaitForFiber(); 
                    FiberManager::Get()->NextFiber();
                }
            }

            // this just switches fibers until we have one owned by this thread,
            // if applicable.
            FiberManager::Get()->FinishFiber();
        }
        



        ////////////////////////////////////////////////////////////////////////////////
        // this is the only function that gets called from a 'client' fiber, the rest
        // are used to implement the service 'update' function.
        void FiberService::EnqueueRequest(void* requestData)
        {
            FiberManager::Get()->YieldFiberToService(this, requestData);
        }
            
        auto FiberService::PopServiceRequest() -> ServiceRequest*
        {
            return m_queue.pop();
        }

        void FiberService::PushServiceFiber(ServiceRequest* request)
        {
            m_queue.push(request);
            Notify();
        }

        void FiberService::CompleteRequest(ServiceRequest* request)
        {
            request->counter->DecRef();
            FiberManager::Get()->NotifyServiceComplete();
        }
            
        void FiberService::Notify()
        {
            if(!m_notified.exchange(true, std::memory_order_acq_rel))
                m_semNotify.Release();
        }

        void FiberService::WaitForUpdate()
        {
            if (!m_notified.exchange(false, std::memory_order_acq_rel))
                m_semNotify.Acquire();
        }

        void FiberService::Start()
        {
            m_thread = lptk::Thread(RunThread, this);
        }
            
        void FiberService::RunThread(FiberService* service)
        {
            while (!service->m_finished.load(std::memory_order_acquire))
            {
                if (!service->Update())
                {
                    service->WaitForUpdate();
                }
            }

            auto request = service->m_queue.pop();
            while (request)
            {
                service->CancelRequest(request);
                service->CompleteRequest(request);
                request = service->m_queue.pop();
            }
        }

        void FiberService::Stop()
        {
            m_finished.exchange(true);
            Notify();

            if(m_thread.joinable())
                m_thread.join();
        }
        
        ////////////////////////////////////////////////////////////////////////////////     
        bool Init(const FiberInitStruct& init)
        {
            return FiberManager::Init(init);
        }

        bool Purge()
        {
            return FiberManager::Purge();
        }
        
        void RunTasks(Task* tasks, size_t numTasks, Counter* counter, int priority)
        {
            FiberManager::Get()->RunTasks(tasks, numTasks, counter, priority);
        }
        
        void WaitForCounter(Counter* counter)
        {
            FiberManager::Get()->WaitForCounter(counter);
        }

        void YieldFiber()
        {
            FiberManager::Get()->YieldFiber();
        }
        
        bool IsInFiberThread()
        {
            return FiberManager::Get()->IsInFiberThread();
        }
        
        int GetFiberThreadId()
        {
            return FiberManager::Get()->GetFiberThreadId();
        }
    }
}