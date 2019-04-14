#pragma once

#include <atomic>

#define LPTK_TASKMGR_INCLUDE_DEBUG_STATUS 0
#define LPTK_TASKMGR_INCLUDE_DEBUG_TRACK_THIEF 0

namespace lptk
{
    namespace task
    {
        ////////////////////////////////////////////////////////////////////////////////
        class Task
        {
        public:
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            enum class Status : int32_t {
                Invalid = -1,
                Created = 0,
                Running,
                Executed,
                Finished,
                Deleted,
            };
#endif

            static constexpr int kCacheLine = 64;
            using TaskFn = void(*)(Task*, const void*, uint32_t dataSize);

            TaskFn Function() const { return m_function; }
            
            void SetData(const void* data, uint32_t n);

        private:
            friend class TaskMgr;
            friend class TaskHandle;
            friend void Init(int, uint32_t);

            const void* GetData() const;
            void FreeData();

            TaskFn m_function = nullptr;
            Task* m_parent = nullptr;
            int32_t m_ownerIndex = -1;
            std::atomic<int32_t> m_unfinished = 1;
            std::atomic<int32_t> m_users = 0;
#if LPTK_TASKMGR_INCLUDE_DEBUG_TRACK_THIEF
            std::atomic<int32_t> m_thief = -1;
#endif
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
            std::atomic<Status> m_status = Status::Invalid;
#endif
            uint32_t m_dataSize = 0;
            
            char m_padding[kCacheLine
                - sizeof(decltype(m_function))
                - sizeof(decltype(m_parent))
                - sizeof(decltype(m_ownerIndex))
                - sizeof(decltype(m_unfinished))
                - sizeof(decltype(m_users))
#if LPTK_TASKMGR_INCLUDE_DEBUG_TRACK_THIEF
                - sizeof(decltype(m_thief))
#endif
#if LPTK_TASKMGR_INCLUDE_DEBUG_STATUS
                - sizeof(decltype(m_status))
#endif
                - sizeof(decltype(m_dataSize))
            ];
            static_assert(sizeof(decltype(m_padding)) >= sizeof(void*), "need to store at least a pointer");
        };
        static_assert(sizeof(Task) == Task::kCacheLine, "Task should fit in kCacheLine");



        ////////////////////////////////////////////////////////////////////////////////
        class TaskHandle
        {
            Task* m_task = nullptr;
        public:
            TaskHandle();
            explicit TaskHandle(Task* task);
            TaskHandle(const TaskHandle& other);
            TaskHandle& operator=(const TaskHandle& other);
            TaskHandle(TaskHandle&& other);
            TaskHandle& operator=(TaskHandle&& other);

            ~TaskHandle();
            void reset(Task* other = nullptr);

            Task* operator->() const { return m_task; }
            Task& operator*() const { return *m_task; }
            Task* get() const { return m_task; }

            bool valid() const { return m_task != nullptr; }
            bool null() const { return m_task == nullptr; }
        };



        ////////////////////////////////////////////////////////////////////////////////
        void Init(int numWorkers, uint32_t logQueueSize);
        void Shutdown();

        TaskHandle CreateTask(Task::TaskFn function);
        TaskHandle CreateChildTask(const TaskHandle& parent, Task::TaskFn function);
        bool Run(const TaskHandle& task);
        void Wait(const TaskHandle& task);

    }
}

