#include "toolkit/taskmgr.hh"

#include <cstdint>

#include "toolkit/thread.hh"
#include "toolkit/dynary.hh"
#include "toolkit/mem.hh"
#include "toolkit/mem/pool_allocator.hh"

namespace lptk
{
    namespace task
    {
        ////////////////////////////////////////////////////////////////////////////////
        static void DeletedTask(Task*, const void*, uint32_t)
        {
            ASSERT(false);
        }
        
        static void FreedTask(Task*, const void*, uint32_t)
        {
            ASSERT(false);
        }

        static void EmptyTask(Task*, const void*, uint32_t)
        {
            ASSERT(false);
        }

        static void AbortedTask(Task*, const void*, uint32_t)
        {
            ASSERT(false);
        }
        
        static Task s_emptyTask;
        static Task s_abortTask;

        static bool IsEmptyTask(const Task* task)
        {
            return task == &s_emptyTask || task == &s_abortTask;
        }

        static void FreeTask(Task* task);
        
        
        
        ////////////////////////////////////////////////////////////////////////////////
        void Task::SetData(const void* data, uint32_t n)
        {
            if (n > sizeof(m_padding))
            {
                auto copy = lptk::mem_allocate(n, MEMPOOL_General, 16);
                if (copy)
                {
                    *reinterpret_cast<void**>(m_padding) = copy;
                    memcpy(copy, data, n);
                    m_dataSize = n;
                }
            }
            else
            {
                memcpy(m_padding, data, n);
                m_dataSize = n;
            }
        }
            
        const void* Task::GetData() const
        {
            if (m_dataSize == 0)
                return nullptr;

            if (m_dataSize > sizeof(m_padding))
            {
                return *reinterpret_cast<const void* const*>(m_padding);
            }
            else
            {
                return &m_padding[0];
            }
        }

        void Task::FreeData()
        {
            if (m_dataSize > sizeof(m_padding))
            {
                auto data = *reinterpret_cast<void**>(m_padding);
                lptk::mem_free(data);
            }
            m_dataSize = 0;
        }


        ////////////////////////////////////////////////////////////////////////////////
        TaskHandle::TaskHandle(){}
        TaskHandle::TaskHandle(Task* task)
            : m_task(task)
        {
            if (m_task)
                m_task->m_users.fetch_add(1, std::memory_order_relaxed);
        }
        TaskHandle::TaskHandle(const TaskHandle& other)
            : m_task(other.m_task)
        {
            if (m_task)
                m_task->m_users.fetch_add(1, std::memory_order_relaxed);
        }
        TaskHandle& TaskHandle::operator=(const TaskHandle& other)
        {
            if (this != &other)
            {
                reset(other.m_task);
            }
            return *this;
        }
        TaskHandle::TaskHandle(TaskHandle&& other)
            : m_task(other.m_task)
        {
            other.m_task = nullptr;
        }
        TaskHandle& TaskHandle::operator=(TaskHandle&& other)
        {
            if (this != &other)
            {
                reset();
                m_task = other.m_task;
                other.m_task = nullptr;
            }
            return *this;
        }

        TaskHandle::~TaskHandle()
        {
            reset();
        }

        void TaskHandle::reset(Task* other)
        {
            if (other)
                other->m_users.fetch_add(1, std::memory_order_relaxed);

            if (m_task)
            {
                const auto unfinished = m_task->m_unfinished.load(std::memory_order_acquire);
                const auto usersRemaining = m_task->m_users.fetch_sub(1, std::memory_order_acq_rel) - 1;
                if (unfinished == 0 && usersRemaining == 0)
                {
                    ASSERT(m_task->m_users.exchange(-1) == 0);
                    FreeTask(m_task);
                }
                m_task = nullptr;
            }

            m_task = other;
        }


        ////////////////////////////////////////////////////////////////////////////////
        class CircularBuffer
        {
            unsigned int m_logSize;
            std::unique_ptr<Task*[]> m_storage;

        public:
            unsigned int capacity() const { return 1u << m_logSize; }
            CircularBuffer(unsigned int logSize)
                : m_logSize(logSize)
                , m_storage(new Task*[capacity()])
            {
                memset(m_storage.get(), 0, capacity() * sizeof(*m_storage.get()));
            }

            Task* Get(uint64_t loc)
            {
                return m_storage[loc % capacity()];
            }

            void Put(uint64_t loc, Task* t)
            {
                m_storage[loc % capacity()] = t;
            }
        };



        ////////////////////////////////////////////////////////////////////////////////
        class WorkQueue
        {
            std::atomic<int64_t> m_top;    // shared
            std::atomic<int64_t> m_bottom; // queue thread owned
            CircularBuffer m_storage;
        public:
            WorkQueue(unsigned int logSize);
            ~WorkQueue();

            // queue owner function
            Task* Pop();
            bool Push(Task* item);
           
            // shared function
            Task* Steal();

        };

        WorkQueue::WorkQueue(unsigned int logSize)
            : m_storage(logSize)
        {
            m_top = 0;
            m_bottom = 0;
        }

        WorkQueue::~WorkQueue()
        {
        }
        
        bool WorkQueue::Push(Task* item)
        {
            auto bottom = m_bottom.load(std::memory_order_acquire);
            const auto top = m_top.load(std::memory_order_acquire);
            const auto size = bottom - top;

            ASSERT(bottom >= top);
            
            if (size >= m_storage.capacity() - 1)
                return false;

            m_storage.Put(bottom++, item);
            m_bottom.store(bottom, std::memory_order_release);
            return true;
        }

        // queue owner functions
        Task* WorkQueue::Pop()
        {
            // write this first because we're now using that index. 
            const auto bottom = m_bottom.load(std::memory_order_acquire) - 1;

            // using seq_cst here to prevent the acquire for top from being moved
            // above this store. 
            m_bottom.store(bottom, std::memory_order_seq_cst);

            const auto top = m_top.load(std::memory_order_acquire);
            const auto size = bottom - top;

            if (size < 0)
            {
                m_bottom.store(top, std::memory_order_release);
                return &s_emptyTask;
            }

            Task* item = m_storage.Get(bottom);
            if (size > 0)
            {
                return item;
            }

            // otherwise size was 0 and we're about to empty the queue. 
            // A stealing thread might be looking at the same top index.
            auto oldTop = top;
            if (!m_top.compare_exchange_strong(oldTop, top + 1,
                std::memory_order_seq_cst, std::memory_order_relaxed))
            {
                item = &s_abortTask;
            }
            m_bottom.store(top + 1, std::memory_order_release);

            // we incremented oldTop before the stealing thread did, so we get to return the item.
            // ensure that we are empty by modifying bottom
            return item;
        }

        Task* WorkQueue::Steal()
        {
            // getting top first means that any Pop() that occurerd must have
            // already decremented bottom. If we reverse the order, the value of
            // bottom could be out of date.
            const auto top = m_top.load(std::memory_order_acquire);
            const auto bottom = m_bottom.load(std::memory_order_acquire);
            const auto size = bottom - top;
            if (size <= 0)
                return &s_emptyTask;

            Task* item = m_storage.Get(top);
            auto oldTop = top;
            if (!m_top.compare_exchange_strong(oldTop, top + 1,
                std::memory_order_seq_cst, std::memory_order_relaxed))
            {
                return &s_abortTask;
            }
            return item;
        }

        

        ////////////////////////////////////////////////////////////////////////////////
        class TaskMgr
        {
        public:
            TaskMgr(int numThreads, uint32_t logQueueSize);
            ~TaskMgr();

            Task* CreateTask(Task::TaskFn function);
            Task* CreateChildTask(Task* parent, Task::TaskFn function);
            bool Run(Task* task);
            void Wait(Task* task);
            void FreeTask(Task* task);

        private:
            static void WorkerThread(TaskMgr* taskMgr);
            void Execute(Task* task);
            void Finish(Task* task);
            static bool IsTaskFinished(Task* task);
           
            //////////////////////////////////////////////////////////////////////////////// 
            struct ThreadLocalData
            {
                ThreadLocalData(uint32_t logSize)
                    : m_taskPool(size_t(1) << logSize, sizeof(Task), false)
                    , m_workQueue(logSize)
                    , m_stealIndex(0)
                {
                    m_freeList = nullptr;
                }
                ThreadLocalData(const ThreadLocalData&) = delete;
                ThreadLocalData& operator=(const ThreadLocalData&) = delete;

                lptk::mem::PoolAlloc m_taskPool;
                std::atomic<Task*> m_freeList;
                WorkQueue m_workQueue;
                unsigned int m_stealIndex;
            };

            ThreadLocalData* GetThreadData();
            Task* AllocateTask();
            TaskHandle GetTask();
            WorkQueue* GetWorkQueue();
            WorkQueue* GetStealWorkQueue();
            
            std::atomic<bool> m_done;
            lptk::DynAry< lptk::Thread > m_workers;
            lptk::DynAry< std::unique_ptr<ThreadLocalData> > m_ownerData;
            static thread_local int s_ownerIndex;
        };
            
        thread_local int TaskMgr::s_ownerIndex = -1;
            
        
        
        ////////////////////////////////////////////////////////////////////////////////
        TaskMgr::TaskMgr(int numThreads, uint32_t logQueueSize)
        {
            m_done = false;
            m_workers.reserve(numThreads);
            m_ownerData.resize(numThreads + 1);
            s_ownerIndex = 0;
            m_ownerData[0] = make_unique<ThreadLocalData>(logQueueSize);
            for (auto i = 0; i < numThreads; ++i)
            {
                auto ownerIndex = 1u + i;
                m_ownerData[ownerIndex] = make_unique<ThreadLocalData>(logQueueSize);
            }

            for (auto i = 0; i < numThreads; ++i)
            {
                auto ownerIndex = 1u + i;
                m_workers.emplace_back(lptk::Thread([this, ownerIndex]{
                    s_ownerIndex = ownerIndex;
                    WorkerThread(this);
                }));
            }
        }

        TaskMgr::~TaskMgr()
        {
            m_done.store(true, std::memory_order_release);
            for (auto&& th : m_workers)
            {
                if (th.joinable())
                    th.join();
            }
        }

        TaskMgr::ThreadLocalData* TaskMgr::GetThreadData()
        {
            ASSERT(s_ownerIndex >= 0 &&
                static_cast<unsigned int>(s_ownerIndex) < m_ownerData.size());
            return m_ownerData[s_ownerIndex].get();
        }

        void TaskMgr::WorkerThread(TaskMgr* taskMgr)
        {
            while (!taskMgr->m_done.load(std::memory_order_acquire))
            {
                auto task = taskMgr->GetTask();
                if (task.valid())
                    taskMgr->Execute(task.get());
            }
        }
            
        void TaskMgr::Execute(Task* task)
        {
            ASSERT(task->m_users >= 1);
            (task->m_function)(task, task->GetData(), task->m_dataSize);
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            auto const oldState = task->m_status.exchange(Task::Status::Executed, std::memory_order_acq_rel);
            if (oldState != Task::Status::Running) DEBUGBREAK();
#endif
            Finish(task);
            ASSERT(task->m_users >= 1);
        }
            
        void TaskMgr::Finish(Task* task)
        {
            ASSERT(task->m_users > 0);
            auto taskParent = task->m_parent;
            const auto unfinishedCount = task->m_unfinished.fetch_sub(1, std::memory_order_acq_rel) - 1;

            if (unfinishedCount == 0)
            {
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
                auto const oldState = task->m_status.exchange(Task::Status::Finished, std::memory_order_acq_rel);
                if (oldState != Task::Status::Executed) DEBUGBREAK();
#endif

                if (taskParent)
                    Finish(taskParent);
            }
            ASSERT(task->m_users > 0);
        }
            
        bool TaskMgr::IsTaskFinished(Task* task)
        {
            return task->m_unfinished.load(std::memory_order_acquire) == 0;
        }

        TaskHandle TaskMgr::GetTask()
        {
            auto queue = GetWorkQueue();
            auto task = queue->Pop();
            if (IsEmptyTask(task))
            {
                auto stealQueue = GetStealWorkQueue();
                if (!stealQueue)
                {
                    lptk::YieldThread();
                    return TaskHandle();
                }

                auto stolenTask = stealQueue->Steal();
                if (IsEmptyTask(stolenTask))
                {
                    lptk::YieldThread();
                    return TaskHandle();
                }
           
#if LPTK_TASKMGR_INCLUDE_DEBUG_TRACK_THIEF
                auto prev = stolenTask->m_thief.exchange(s_ownerIndex);
                if (prev != -1) DEBUGBREAK();
#endif
                return TaskHandle(stolenTask);
            }
            else
            {
#if LPTK_TASKMGR_INCLUDE_DEBUG_TRACK_THIEF
                auto prev = task->m_thief.exchange(s_ownerIndex);
                if (prev != -1) DEBUGBREAK();
#endif
                return TaskHandle(task);
            }
        }

        WorkQueue* TaskMgr::GetWorkQueue()
        {
            auto threadData = GetThreadData();
            return &threadData->m_workQueue;
        }

        WorkQueue* TaskMgr::GetStealWorkQueue()
        {
            auto threadData = GetThreadData();
            auto index = static_cast<int32_t>((threadData->m_stealIndex) % m_ownerData.size());
            if (index == s_ownerIndex)
                index = (index + 1) % m_ownerData.size();

            if (index != s_ownerIndex)
            {
                auto queue = &m_ownerData[index]->m_workQueue;
                index = (index + 1) % m_ownerData.size();
                threadData->m_stealIndex = index;
                return queue;
            }

            return nullptr;
        }

        bool TaskMgr::Run(Task* task)
        {
            ASSERT(task != nullptr);
            ASSERT(task->m_users >= 1);
            auto queue = GetWorkQueue();
            // The WorkQueue returns false if there's no room, but we 
            // should never hit that case, because we can only allocate 
            // as many tasks as we have size in the queue 
            // (we should hit a CreateTask failure first)
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            auto const oldState = task->m_status.exchange(Task::Status::Running, std::memory_order_acq_rel);
            if(oldState != Task::Status::Created) DEBUGBREAK();
#endif
            const auto ok = queue->Push(task);
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            if (!ok)
            {
                auto const oldState = task->m_status.exchange(Task::Status::Created, std::memory_order_acq_rel);
                if (oldState != Task::Status::Running) DEBUGBREAK();
            }
#endif
            return ok;
        }

        void TaskMgr::Wait(Task* task)
        {
            ASSERT(task != nullptr);
            ASSERT(task->m_users >= 1);

            while (!IsTaskFinished(task))
            {
                auto nextTask = GetTask();
                if (nextTask.valid())
                {
                    Execute(nextTask.get());
                }
            }

            ASSERT(task->m_users >= 1);
        }


        Task* TaskMgr::AllocateTask()
        {
            // free tasks that belong to this thread
            auto threadData = GetThreadData();
            {
                auto cur = threadData->m_freeList.exchange(nullptr, std::memory_order_acq_rel);
                    
                while (cur)
                {
                    ASSERT(cur->m_ownerIndex == s_ownerIndex);
                    ASSERT(cur->m_unfinished == 0);
                    ASSERT(cur->m_users == -1);
                    auto next = cur->m_parent;
                    cur->m_function = &FreedTask;
                    threadData->m_taskPool.Free(cur);
                    cur = next;
                }
            }

            // allocate the new task from our pool
            auto task = threadData->m_taskPool.Create<Task>();
            if (task)
            {
                task->m_ownerIndex = s_ownerIndex;
            }
            return task;
        }
           
        void TaskMgr::FreeTask(Task* task)
        {
            ASSERT(task != nullptr);
            ASSERT(task->m_unfinished == 0);
            ASSERT(task->m_users == -1);
            ASSERT(task->m_ownerIndex >= 0);
            ASSERT(static_cast<unsigned>(task->m_ownerIndex) < m_ownerData.size());
            ASSERT(task->m_function != &DeletedTask);
            ASSERT(task->m_function != &FreedTask);
            ASSERT(task->m_function != &EmptyTask);
            ASSERT(task->m_function != &AbortedTask);

            task->FreeData();

            const auto ownerIndex = task->m_ownerIndex;
            const auto ownerData = m_ownerData[ownerIndex].get();

#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            auto const oldState = task->m_status.exchange(Task::Status::Deleted, std::memory_order_acq_rel);
            if (oldState != Task::Status::Finished) DEBUGBREAK();
#endif

            if (ownerIndex == s_ownerIndex)
            {
                task->m_function = &FreedTask;
                ownerData->m_taskPool.Destroy(task);
            }
            else
            {
                task->m_function = &DeletedTask;
                // The ideal code here would be:
                //   task->m_parent = ownerData->m_freeList.load(std::memory_order_relaxed);
                //   while (!ownerData->m_freeList.compare_exchange_weak(task->m_parent, task));
                // but we do this instead because of a bug in vs2013. see bug: 819819 
                // this fixes by keeping a copy of oldHead for the expected value, which can't be seen
                // by another thread (it's not part of the linked list); it is only written to
                // by init and the CAS call.
                auto oldHead = ownerData->m_freeList.load(std::memory_order_relaxed);
                do {
                    task->m_parent = oldHead;
                } while (!ownerData->m_freeList.compare_exchange_weak(oldHead, task, 
                    std::memory_order_release, std::memory_order_relaxed));
            }
        }

        Task* TaskMgr::CreateTask(Task::TaskFn function)
        {
            auto task = AllocateTask();
            if (!task)
                return task;
            task->m_function = function;
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            auto const oldState = task->m_status.exchange(Task::Status::Created, std::memory_order_acq_rel);
            if (oldState != Task::Status::Invalid) DEBUGBREAK();
#endif
            return task;
        }

        Task* TaskMgr::CreateChildTask(Task* parent, Task::TaskFn function)
        {
            auto task = AllocateTask();
            if (!task)
                return task;
            
            parent->m_unfinished.fetch_add(1, std::memory_order_acq_rel);

            task->m_function = function;
            task->m_parent = parent;
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            auto const oldState = task->m_status.exchange(Task::Status::Created, std::memory_order_acq_rel);
            if (oldState != Task::Status::Invalid) DEBUGBREAK();
#endif
            return task;
        }


        //////////////////////////////////////////////////////////////////////////////// 
        // public C-style interface functions
        static std::unique_ptr<TaskMgr> g_taskMgr;
        void Init(int numWorkers, uint32_t logQueueSize)
        {
            ASSERT(g_taskMgr.get() == nullptr);
            s_abortTask.m_function = &AbortedTask;
            s_emptyTask.m_function = &EmptyTask;
            g_taskMgr = make_unique<TaskMgr>(numWorkers, logQueueSize);
        }

        void Shutdown()
        {
            ASSERT(g_taskMgr.get() != nullptr);
            g_taskMgr.reset();
        }

        TaskHandle CreateTask(Task::TaskFn function)
        {
            return TaskHandle(g_taskMgr->CreateTask(function));
        }

        TaskHandle CreateChildTask(const TaskHandle& parent, Task::TaskFn function)
        {
            return TaskHandle(g_taskMgr->CreateChildTask(parent.get(), function));
        }

        bool Run(const TaskHandle& task)
        {
            return g_taskMgr->Run(task.get());
        }

        void Wait(const TaskHandle& task)
        {
            g_taskMgr->Wait(task.get());
        }

        static void FreeTask(Task* task)
        {
            g_taskMgr->FreeTask(task);
        }
    }

}
