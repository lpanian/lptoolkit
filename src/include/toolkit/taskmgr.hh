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
            using TaskFn = void(*)(Task*, const void*);

            TaskFn m_function;
            Task* m_parent;
            std::atomic<int32_t> m_unfinished;
            std::atomic<int32_t> m_users;
            int32_t m_ownerIndex;

            char padding[kCacheLine
                - sizeof(decltype(m_function))
                - sizeof(decltype(m_parent))
                - sizeof(decltype(m_unfinished))
                - sizeof(decltype(m_users))
                - sizeof(decltype(m_ownerIndex))
            ];
        };
        static_assert(sizeof(Task) == Task::kCacheLine, "Task should fit in kCacheLine");



        ////////////////////////////////////////////////////////////////////////////////
        class TaskHandle
        {
            Task* m_task = nullptr;
        public:
            TaskHandle();
            TaskHandle(Task* task);
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
