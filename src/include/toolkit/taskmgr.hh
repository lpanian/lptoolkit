#pragma once
#ifndef INCLUDED_TOOLKIT_TASKMGR_HH
#define INCLUDED_TOOLKIT_TASKMGR_HH

#include <atomic>

namespace lptk
{
    namespace task
    {
        ////////////////////////////////////////////////////////////////////////////////
        class Task
        {
        public:
            static constexpr int kCacheLine = 64;
            using TaskFn = void(*)(Task*, const void*, uint32_t dataSize);

            TaskFn Function() const { return m_function; }
            
            void SetData(const void* data, uint32_t n);

        private:
            friend class TaskMgr;
            friend class TaskHandle;
            friend void Init(int);

            const void* GetData() const;
            void FreeData();

            TaskFn m_function = nullptr;
            Task* m_parent = nullptr;
            int32_t m_ownerIndex = -1;
            std::atomic<int32_t> m_unfinished = 1;
            std::atomic<int32_t> m_users = 0;
            uint32_t m_dataSize = 0;

            char m_padding[kCacheLine
                - sizeof(decltype(m_function))
                - sizeof(decltype(m_parent))
                - sizeof(decltype(m_ownerIndex))
                - sizeof(decltype(m_unfinished))
                - sizeof(decltype(m_users))
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
        void Init(int numWorkers);
        void Shutdown();

        TaskHandle CreateTask(Task::TaskFn function);
        TaskHandle CreateChildTask(const TaskHandle& parent, Task::TaskFn function);
        bool Run(const TaskHandle& task);
        void Wait(const TaskHandle& task);

    }
}

#endif
