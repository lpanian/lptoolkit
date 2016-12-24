#include "toolkit/fiber.hh"

#include "toolkit/thread.hh"
#include "toolkit/dynary.hh"

#include "toolkit/spinlockqueue.hh"
#include "toolkit/circularqueue.hh"

/*

When tasks are kicked, they are placed in one of two queues, 
either the normal priority or high priority queues. These queues can
grow while we have available memory.

When high priority tasks exist, worker theads will try to grab these first.
Otherwise, they grab a normal task. There may be some number of threads dedicated
to high priority tasks only, but this is not necessarily true.

Each thread has a dedicated pool of fibers it can be currently executing. It may
have up to two different sizes of fibers, one large stack and one small stack.

The worker first attempts to grab a task. We do this first because we have multiple fiber
pools to draw from, and we need to know which one to check. We first try to pop a high priority task,
and if that is null, then we try to pop a low priority task.

If we have a task, then try to acquire a fiber of the correct stack size. If we cannot, continue
to wait check. If we have one, then initialize the fiber to that task and switch to the selected 
fiber.

Each worker thread's main execution loop checks that it has an available fiber. If it does not, all
fibers must be in a waiting state (waiting on a fiber service). In this case, do not pull
a new task, and go into a waiting state. When a fiber service completes, it will 
notify the fiber's owning thread, restarting the update loop. 

*/

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



        ////////////////////////////////////////////////////////////////////////////////
        class Fiber
        {
            friend class FiberManager;
            friend struct FiberNodeTraits;
        public:
            Fiber();

            Fiber(const Fiber&) = delete;
            Fiber& operator=(const Fiber&) = delete;
            Fiber(Fiber&& other) = delete;
            Fiber& operator=(Fiber&& other) = delete;

            ~Fiber();

            bool Init(int threadIndex, unsigned stackSize, FiberPool* ownerPool);

            int GetThreadIndex() const { return m_threadIndex; }
            void* GetServiceData() const { return m_serviceData; }
            FiberPool* GetOwnerPool() const { return m_ownerPool; }
            
            void Continue();

            bool IsTaskComplete() const;
            bool IsWaiting() const;

            void SetTask(Task* task) {
                ASSERT(m_task == nullptr);
                m_task = task;
            }
        private:
            void Run();

            Task* m_task = nullptr;
            void* m_serviceData = nullptr;

            int m_threadIndex = -1;
            FiberPool* m_ownerPool = nullptr;
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
        class FiberPool
        {
        public:
            FiberPool();

            FiberPool(const FiberPool&) = delete;
            FiberPool& operator=(const FiberPool&) = delete;
            FiberPool(FiberPool&&) = default;
            FiberPool& operator=(FiberPool&&) = default;

            ~FiberPool();

            bool Init(int threadIndex, unsigned numFibers, unsigned stackSize);
            Fiber* Acquire();
            void Release(Fiber* fiber);

            bool Empty() const { return m_availableFibers.empty(); }
        private:
            lptk::DynAry< std::unique_ptr<Fiber> > m_allFibers;
            lptk::DynAry< Fiber* > m_availableFibers;
            int m_threadIndex = -1;
            unsigned m_stackSize = 0;
        };
            
        FiberPool::FiberPool() {}
        FiberPool::~FiberPool() {}

        bool FiberPool::Init(int threadIndex, unsigned numFibers, unsigned stackSize)
        {
            if (m_threadIndex >= 0)
                return false;

            m_threadIndex = threadIndex;
            m_allFibers.reserve(numFibers);
            m_availableFibers.reserve(numFibers);

            for (unsigned i = 0; i < numFibers; ++i)
            {
                m_allFibers.push_back(make_unique<Fiber>());
                if (!m_allFibers.back()->Init(threadIndex, stackSize, this))
                {
                    return false;
                }
                m_availableFibers.push_back(m_allFibers.back().get());
            }

            m_stackSize = stackSize;
            return true;
        }

        Fiber* FiberPool::Acquire()
        {
            if (!m_availableFibers.empty())
            {
                auto result = m_availableFibers.back();
                m_availableFibers.pop_back();
                return result;
            }
            return nullptr;
        }

        void FiberPool::Release(Fiber* fiber)
        {
            ASSERT(fiber->GetThreadIndex() == m_threadIndex);
            m_availableFibers.push_back(fiber);
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
        
            void RunTasks(Task* tasks, size_t numTasks, Counter* counter);
            void RunHighPriorityTasks(Task* tasks, size_t numTasks, Counter* counter);
            
            void YieldFiber();
            void YieldFiberToService(FiberService* service, void* requestData);
            void CompleteFiberService(Fiber* fiber);

            Fiber* GetCurrentFiber() const;

            bool IsExitRequested() const;
            
            void ScheduleOneFiber();
        private:
            bool InitMain(const FiberInitStruct& init);

            static void WorkerMain(FiberManager* fiberMgr, unsigned threadIndex);

            std::atomic<bool> m_exitRequested = false;

            struct ThreadData
            {
                int m_priority = 0;
                void* m_mainFiber = nullptr;
                Fiber* m_currentFiber = nullptr;
                FiberPool m_smallStackPool;
                FiberPool m_largeStackPool;
                std::unique_ptr<CircularQueue<Fiber*>> m_executing;
            };
            
            lptk::DynAry<lptk::Thread> m_workerThreads;
            lptk::DynAry<ThreadData> m_threadData;

            lptk::IntrusiveSpinLockQueue<Task> m_lowPriorityTaskQueue;
            lptk::IntrusiveSpinLockQueue<Task> m_highPriorityTaskQueue;

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
            if (!s_ptr)
                return false;
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
            auto& threadData = fiberMgr->m_threadData[threadIndex];

            // initial set up
#if defined(WINDOWS)
            threadData.m_mainFiber = ConvertThreadToFiber(nullptr);
#elif defined(LINUX)
#error linux version of FiberManager::InitThread not yet implemented
#endif

            // main scheduling loop
            while (!fiberMgr->m_exitRequested.load(std::memory_order_relaxed))
            {
                // TODO: this thread can sleep if there are no executing fibers and no 
                // tasks available. Do a lazy check followed by a cv block here in those cases
                // and notify when executing list changes or tasks are pushed

                fiberMgr->ScheduleOneFiber();
            }

        }
            
        void FiberManager::ScheduleOneFiber()
        {
            auto& threadData = m_threadData[s_currentThread];
            // attempt to schedule a new task
            if (!threadData.m_largeStackPool.Empty() || !threadData.m_smallStackPool.Empty())
            {
                // is there a high priority task we can schedule? otherwise grab a low
                // priority task, unless this is a high priority-only thread.
                auto task = m_highPriorityTaskQueue.pop();
                int taskPriority = 1;
                if (!task && threadData.m_priority < 1)
                {
                    task = m_lowPriorityTaskQueue.pop();
                    taskPriority = 0;
                }

                // attempt to allocate a fiber for our task
                if (task)
                {
                    Fiber* fiber = nullptr;
                    if (task->IsLargeStackTask())
                    {
                        fiber = threadData.m_largeStackPool.Acquire();
                    }
                    else
                    {
                        fiber = threadData.m_smallStackPool.Acquire();
                    }

                    if (!fiber)
                    {
                        // hmm, all of our fibers for the given stack size are in use. Requeue this task.
                        if (taskPriority == 1)
                            m_highPriorityTaskQueue.push(task);
                        else
                            m_lowPriorityTaskQueue.push(task);
                        task = nullptr;
                    }
                    else
                    {
                        fiber->SetTask(task);
                        const auto result = threadData.m_executing->push(fiber);
                        ASSERT(result);
                    }
                }

            }

            // choose a fiber to execute/resume based on what is available in executing fibers.
            Fiber* fiber = nullptr;
            if (threadData.m_executing->pop(fiber))
            {
                threadData.m_currentFiber = fiber;
                fiber->Continue();
                threadData.m_currentFiber = nullptr;
                if (fiber->IsTaskComplete())
                {
                    // the task is finished, and the fiber can be returned to our fiber pools
                    auto pool = fiber->GetOwnerPool();
                    pool->Release(fiber);
                }
                else if (!fiber->IsWaiting())
                {
                    // the fiber has yielded but is not finished with its task, and is not currently
                    // waiting on a blocking fiber service. So, return it to the executing list.
                    threadData.m_executing->push(fiber);
                }
                else
                {
                    // do nothing - if we need to keep track of fibers that are waiting
                    // on a fiber service, do that here
                }
            }

        }
            
        bool FiberManager::InitMain(const FiberInitStruct& init)
        {
            const unsigned numLowPriorityWorkers = lptk::Max(1u, init.numWorkerThreads);
            const unsigned numHighPriorityWorkers = init.numHighPriorityWorkerThreads;
            const unsigned numWorkers = numLowPriorityWorkers + numHighPriorityWorkers;

            m_workerThreads.reserve(numWorkers-1);
            m_threadData.resize(numWorkers);
            for (unsigned i = 0; i < numWorkers; ++i)
            {
                auto& threadData = m_threadData[i];
                threadData.m_priority = i < numLowPriorityWorkers ? 0 : 1;
                const auto numSmallFibers = lptk::Max(1u, threadData.m_priority == 0 ?
                    init.numSmallFibersPerThread :
                    init.numSmallFibersPerHighPriorityThread);
                if (!threadData.m_smallStackPool.Init(int(i), numSmallFibers, init.smallFiberStackSize))
                {
                    return false;
                }

                const auto numLargeFibers = lptk::Max(1u, threadData.m_priority == 0 ?
                    init.numLargeFibersPerThread :
                    init.numLargeFibersPerHighPriorityThread);
                if (!threadData.m_largeStackPool.Init(int(i), numSmallFibers, init.largeFiberStackSize))
                {
                    return false;
                }

                threadData.m_executing = make_unique<CircularQueue<Fiber*>>(numSmallFibers + numLargeFibers);

                if (i == 0) // main thread gets to be a fiber too!
                {
                    s_currentThread = 0;
#if defined(WINDOWS)
                    threadData.m_mainFiber = ConvertThreadToFiber(nullptr);
#elif defined(LINUX)
#error linux version of FiberManager::InitThread not yet implemented
#endif
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
            m_exitRequested = true;
            for (auto&& thread : m_workerThreads)
            {
                if (thread.joinable())
                    thread.join();
            }
        }
            
        Fiber* FiberManager::GetCurrentFiber() const
        {
            ASSERT(s_currentThread >= 0 && s_currentThread < int(m_threadData.size()));
            auto& threadData = m_threadData[s_currentThread];

            return threadData.m_currentFiber;
        }
            
        void FiberManager::RunTasks(Task* tasks, size_t numTasks, Counter* counter)
        {
            if (numTasks == 0)
                return;
            ASSERT(counter != nullptr);
            counter->IncRef(numTasks);
            for (size_t i = 0; i < numTasks; ++i)
                tasks[i].SetCounter(counter);
            m_lowPriorityTaskQueue.push_range(tasks, numTasks);
        }

        void FiberManager::RunHighPriorityTasks(Task* tasks, size_t numTasks, Counter* counter)
        {
            if (numTasks == 0)
                return;
            ASSERT(counter != nullptr);
            counter->IncRef(numTasks);
            for (size_t i = 0; i < numTasks; ++i)
                tasks->SetCounter(counter);
            m_highPriorityTaskQueue.push_range(tasks, numTasks);
        }
            
        void FiberManager::YieldFiber()
        {
            const auto fiber = GetCurrentFiber();
            ASSERT(fiber != nullptr);

            const auto threadIndex = fiber->m_threadIndex;
            ASSERT(threadIndex >= 0 && threadIndex < int(m_threadData.size()));
            auto&& threadData = m_threadData[threadIndex];

#if defined (WINDOWS)
            SwitchToFiber(threadData.m_mainFiber);
#else
            static_assert(false, "implement this function for this OS");
#endif
        }
            
        void FiberManager::YieldFiberToService(FiberService* service, void* requestData)
        {
            const auto fiber = GetCurrentFiber();
            ASSERT(fiber != nullptr);

            const auto threadIndex = fiber->m_threadIndex;
            ASSERT(threadIndex >= 0 && threadIndex < int(m_threadData.size()));
            auto&& threadData = m_threadData[threadIndex];

            // we set this before pushing the service fiber so it has all the data it needs to begin
            ASSERT(fiber->m_serviceData == nullptr);
            fiber->m_serviceData = requestData;
            service->PushServiceFiber(fiber);

#if defined (WINDOWS)
            SwitchToFiber(threadData.m_mainFiber);
#else
            static_assert(false, "implement this function for this OS");
#endif
        }
            
        void FiberManager::CompleteFiberService(Fiber* fiber)
        {
            ASSERT(fiber && fiber->m_serviceData);
            fiber->m_serviceData = nullptr;
            
            const auto threadIndex = fiber->m_threadIndex;
            ASSERT(threadIndex >= 0 && threadIndex < int(m_threadData.size()));
            auto&& threadData = m_threadData[threadIndex];

            threadData.m_executing->push(fiber);
        }
            
        bool FiberManager::IsExitRequested() const
        {
            return m_exitRequested.load(std::memory_order_relaxed);
        }
       


        ////////////////////////////////////////
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
            
        bool Fiber::Init(int threadIndex, unsigned stackSize, FiberPool* ownerPool)
        {
            if (m_threadIndex >= 0)
                return false;
            m_threadIndex = threadIndex;

#if defined(WINDOWS)
            m_fiber = CreateFiber(stackSize, &FiberMain, this);
#endif
            m_ownerPool = ownerPool;
            if (m_fiber == false)
                return false;
            return true;
        }

#if defined(WINDOWS)
        void CALLBACK Fiber::FiberMain(void* param)
        {
            auto fiber = reinterpret_cast<Fiber*>(param);
            fiber->Run();
        }
#endif
        
        void Fiber::Continue()
        {
            if (m_fiber)
            {
                SwitchToFiber(m_fiber);
            }
        }
            
        bool Fiber::IsTaskComplete() const
        {
            return m_task == nullptr;
        }

        bool Fiber::IsWaiting() const
        {
            return m_task && m_serviceData;
        }

        void Fiber::Run()
        {
            while (!FiberManager::Get()->IsExitRequested())
            {
                if (m_task)
                {
                    auto&& fn = m_task->GetFunc();
                    auto&& userData = m_task->GetUserData();
                    fn(userData);
                    m_task->m_counter->DecRef();
                    m_task = nullptr;
                    FiberManager::Get()->YieldFiber();
                }
            }
        }
        



        ////////////////////////////////////////////////////////////////////////////////
        // this is the only function that gets called from a 'client' fiber, the rest
        // are used to implement the service 'update' function.
        void FiberService::EnqueueRequest(void* requestData)
        {
            FiberManager::Get()->YieldFiberToService(this, requestData);
        }
            
        Fiber* FiberService::PopServiceFiber()
        {
            return m_queue.pop();
        }

        void FiberService::PushServiceFiber(Fiber* fiber)
        {
            m_queue.push(fiber);
            Notify();
        }

        void FiberService::CompleteRequest(Fiber* fiber)
        {
            FiberManager::Get()->CompleteFiberService(fiber);
        }
            
        void* FiberService::GetFiberServiceData(Fiber* fiber)
        {
            return fiber->GetServiceData();
        }
            
        void FiberService::Notify()
        {
            {
                std::lock_guard<std::mutex> lock(m_updateMutex);
                m_updateRequested = true;
            }
            m_isWaiting.notify_all();
        }

        void FiberService::WaitForUpdate()
        {
            if (!m_updateRequested)
            {
                std::unique_lock<std::mutex> lock(m_updateMutex);
                m_isWaiting.wait(lock, [this] {
                    if (m_updateRequested)
                    {
                        m_updateRequested = false;
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                });
            }
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

            auto fiber = service->m_queue.pop();
            while (fiber)
            {
                service->CancelRequest(fiber);
                service->CompleteRequest(fiber);
                fiber = service->m_queue.pop();
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
        
        void RunTasks(Task* tasks, size_t numTasks, Counter* counter)
        {
            FiberManager::Get()->RunTasks(tasks, numTasks, counter);
        }
        
        void RunHighPriorityTasks(Task* tasks, size_t numTasks, Counter* counter)
        {
            FiberManager::Get()->RunHighPriorityTasks(tasks, numTasks, counter);
        }
        
        void WaitForCounter(Counter* counter)
        {
            while (!counter->IsZero())
            {
                FiberManager::Get()->ScheduleOneFiber();
            }
        }

        void YieldFiber()
        {
            FiberManager::Get()->YieldFiber();
        }
    }
}