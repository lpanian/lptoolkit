#include "toolkit/taskmgr.hh"

#include <cstdint>

#include "toolkit/thread.hh"
#include "toolkit/dynary.hh"
#include "toolkit/mem.hh"
#include "toolkit/poolmem.hh"


namespace lptk
{
    // 1) Init thread local data
    // 2) lock-free allocate/deallocate tasks
    // 3) workqueue for each thread
    // 4) run/wait
    namespace task
    {
        ////////////////////////////////////////////////////////////////////////////////
        static void DeletedTask(Task*, const void*)
        {
            ASSERT(false);
        }

        static void EmptyTask(Task*, const void*)
        {
            ASSERT(false);
        }

        static void AbortedTask(Task*, const void*)
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
        TaskHandle::TaskHandle(){}
        TaskHandle::TaskHandle(Task* task)
            : m_task(task)
        {
            if (m_task)
                m_task->m_users.fetch_add(1, std::memory_order_seq_cst);
        }
        TaskHandle::TaskHandle(const TaskHandle& other)
            : m_task(other.m_task)
        {
            if (m_task)
                m_task->m_users.fetch_add(1, std::memory_order_seq_cst);
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
        {
            reset(other.m_task);
            other.reset();
        }
        TaskHandle& TaskHandle::operator=(TaskHandle&& other)
        {
            if (this != &other)
            {
                reset(other.m_task);
                other.reset();
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
                other->m_users.fetch_add(1, std::memory_order_seq_cst);

            if (m_task)
            {
                auto remaining = m_task->m_users.fetch_sub(1, std::memory_order_seq_cst) - 1;
                if (remaining == 0)
                {
                    auto unfinished = m_task->m_unfinished.load(std::memory_order_seq_cst);
                    if (unfinished == 0)
                    {
                        FreeTask(m_task);
                    }
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
            auto bottom = m_bottom.load(std::memory_order_seq_cst);
            const auto top = m_top.load(std::memory_order_seq_cst);
            const auto size = bottom - top;

            ASSERT(bottom >= top);
            
            if (size >= m_storage.capacity() - 1)
                return false;

            m_storage.Put(bottom++, item);
            m_bottom.store(bottom, std::memory_order_seq_cst);
            return true;
        }

        // queue owner functions
        Task* WorkQueue::Pop()
        {
            // write this first because we're now using that index. 
            const auto bottom = m_bottom.fetch_sub(1, std::memory_order_seq_cst) - 1;
            const auto top = m_top.load(std::memory_order_seq_cst);
            const auto size = bottom - top;

            if (size < 0)
            {
                m_bottom.store(top, std::memory_order_seq_cst);
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

            m_bottom.store(top + 1, std::memory_order_seq_cst);

            // we incremented oldTop before the stealing thread did, so we get to return the item.
            // ensure that we are empty by modifying bottom
            return item;
        }

        Task* WorkQueue::Steal()
        {
            // getting top first means that any Pop() that occurerd must have
            // already decremented bottom. If we reverse the order, the value of
            // bottom could be out of date.
            const auto top = m_top.load(std::memory_order_seq_cst);
            const auto bottom = m_bottom.load(std::memory_order_seq_cst);
            const auto size = bottom - top;
            if (size <= 0)
                return &s_emptyTask;

            Task* item = m_storage.Get(top);
            auto oldTop = top;
            if (!m_top.compare_exchange_strong(oldTop, oldTop + 1,
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
            TaskMgr(int numThreads);
            ~TaskMgr();

            Task* CreateTask(Task::TaskFn function);
            Task* CreateChildTask(Task* parent, Task::TaskFn function);
            void Run(Task* task);
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
                static constexpr uint32_t kLogSize = 12u;
                ThreadLocalData()
                    : m_taskPool(1u << kLogSize, sizeof(Task), false)
                    , m_workQueue(kLogSize)
                    , m_stealIndex(0)
                {
                    m_freeList = nullptr;
                }
                ThreadLocalData(const ThreadLocalData&) = delete;
                ThreadLocalData& operator=(const ThreadLocalData&) = delete;

                lptk::PoolAlloc m_taskPool;
                std::atomic<Task*> m_freeList;
                WorkQueue m_workQueue;
                unsigned int m_stealIndex;
            };

            ThreadLocalData* GetThreadData();
            Task* AllocateTask();
            Task* GetTask();
            WorkQueue* GetWorkQueue();
            WorkQueue* GetStealWorkQueue();
            
            std::atomic<bool> m_done;
            lptk::DynAry< lptk::Thread > m_workers;
            lptk::DynAry< std::unique_ptr<ThreadLocalData> > m_ownerData;
            static thread_local int s_ownerIndex;
        };
            
        thread_local int TaskMgr::s_ownerIndex = -1;
            
        
        
        ////////////////////////////////////////////////////////////////////////////////
        TaskMgr::TaskMgr(int numThreads)
        {
            m_done = false;
            m_workers.reserve(numThreads);
            m_ownerData.resize(numThreads + 1);
            s_ownerIndex = 0;
            m_ownerData[0] = make_unique<ThreadLocalData>();
            for (auto i = 0; i < numThreads; ++i)
            {
                auto ownerIndex = 1u + i;
                m_ownerData[ownerIndex] = make_unique<ThreadLocalData>();
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
            // add null tasks until we shut down? how to deal with work stealing?
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
                if (task)
                    taskMgr->Execute(task);
            }
        }
            
        void TaskMgr::Execute(Task* task)
        {
            (task->m_function)(task, nullptr); // task->m_data);
            Finish(task);
        }
            
        void TaskMgr::Finish(Task* task)
        {
            const auto remainingCount = task->m_unfinished.fetch_sub(1, std::memory_order_seq_cst) - 1;
            if (remainingCount == 0)
            {
                if (task->m_parent)
                {
                    Finish(task->m_parent);
                }

                const auto usersRemaining = task->m_users.load(std::memory_order_seq_cst);
                if (usersRemaining == 0)
                {
                    FreeTask(task);
                }
            }
        }
            
        bool TaskMgr::IsTaskFinished(Task* task)
        {
            return task->m_unfinished.load(std::memory_order_seq_cst) == 0;
        }

        Task* TaskMgr::GetTask()
        {
            auto queue = GetWorkQueue();
            auto task = queue->Pop();
            if (IsEmptyTask(task))
            {
                auto stealQueue = GetStealWorkQueue();
                if (!stealQueue)
                {
                    lptk::YieldThread();
                    return nullptr;
                }

                auto stolenTask = stealQueue->Steal();
                if (IsEmptyTask(stolenTask))
                {
                    lptk::YieldThread();
                    return nullptr;
                }
                return stolenTask;
            }
            else
            {
                return task;
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

        void TaskMgr::Run(Task* task)
        {
            ASSERT(task != nullptr);
            auto queue = GetWorkQueue();
            // The WorkQueue returns false if there's no room, but we 
            // should never hit that case, because we can only allocate 
            // as many tasks as we have size in the queue.
            const auto result = queue->Push(task);
            lptk::unused_arg(result);
            ASSERT(result != false);
        }

        void TaskMgr::Wait(Task* task)
        {
            ASSERT(task != nullptr);
            ASSERT(task->m_users >= 1);

            while (!IsTaskFinished(task))
            {
                auto nextTask = GetTask();
                if (nextTask)
                {
                    Execute(nextTask);
                }
            }
        }

        Task* TaskMgr::AllocateTask()
        {
            // free tasks that belong to this thread
            auto threadData = GetThreadData();
            auto freeList = threadData->m_freeList.exchange(nullptr);
            while (freeList)
            {
                auto next = freeList->m_parent;
                threadData->m_taskPool.Free(freeList);
                freeList = next;
            }

            // allocate the new task from our pool
            auto task = reinterpret_cast<Task*>(threadData->m_taskPool.Alloc());
            if (task)
            {
                memset(task, 0, sizeof(*task));
                task->m_ownerIndex = s_ownerIndex;
            }
            return task;
        }
           
        void TaskMgr::FreeTask(Task* task)
        {
            ASSERT(task && 
                task->m_unfinished == 0 && task->m_users == 0);
            ASSERT(task->m_ownerIndex >= 0 && 
                static_cast<unsigned>(task->m_ownerIndex) < m_ownerData.size());

            auto const ownerIndex = task->m_ownerIndex;
            memset(task, 0, sizeof(*task));
            task->m_ownerIndex = -1;
            task->m_function = &DeletedTask;

            if (ownerIndex == s_ownerIndex)
            {
                auto threadData = GetThreadData();
                threadData->m_taskPool.Free(task);
            }
            else
            {
                auto ownerData = m_ownerData[ownerIndex].get();
                task->m_parent = ownerData->m_freeList.load(std::memory_order_seq_cst);
                while (!ownerData->m_freeList.compare_exchange_weak(task->m_parent, task));
            }
        }

        Task* TaskMgr::CreateTask(Task::TaskFn function)
        {
            auto task = AllocateTask();
            if (!task)
                return task;
            task->m_function = function;
            task->m_parent = nullptr;
            task->m_unfinished = 1;
            task->m_users = 0;
            return task;
        }

        Task* TaskMgr::CreateChildTask(Task* parent, Task::TaskFn function)
        {
            auto task = AllocateTask();
            if (!task)
                return task;
            
            parent->m_unfinished.fetch_add(1, std::memory_order_seq_cst);

            task->m_function = function;
            task->m_parent = parent;
            task->m_unfinished = 1;
            task->m_users = 0;
            return task;
        }


        //////////////////////////////////////////////////////////////////////////////// 
        // public C-style interface functions
        static std::unique_ptr<TaskMgr> g_taskMgr;
        void Init(int numWorkers)
        {
            ASSERT(g_taskMgr.get() == nullptr);
            s_abortTask.m_function = &AbortedTask;
            s_emptyTask.m_function = &EmptyTask;
            g_taskMgr = make_unique<TaskMgr>(numWorkers);
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

        void Run(const TaskHandle& task)
        {
            g_taskMgr->Run(task.get());
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
