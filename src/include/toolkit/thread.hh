#pragma once
#ifndef INCLUDED_toolkit_thread_hh
#define INCLUDED_toolkit_thread_hh

#include <utility>
#include <tuple>
#include <memory>
#if defined(LINUX)
#include <pthread.h>
#endif

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
class Thread
{
public:
    struct ThreadImplBase;
    typedef std::unique_ptr<ThreadImplBase> ImplPtr;

    Thread(); // doesn't do anything, just empty
    Thread(Thread&&); // move
    Thread& operator=(Thread&&);
    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    template<class Fn, class... ArgType>
        explicit Thread(Fn&& f, ArgType&&... args);
    
    ~Thread();

    void swap(Thread& other);
    bool joinable() ;
    void join();

    ////////////////////////////////////////	
    struct ThreadImplBase
    {
        std::unique_ptr<ThreadImplBase> m_this;
        inline virtual ~ThreadImplBase() {}
        virtual void Run() = 0;
    };

    template<typename Callable, typename... Args>
        struct ThreadImpl : public ThreadImplBase
    {
        Callable m_func;
        std::tuple<Args...> m_args;
        ThreadImpl(Callable&& f, Args&&... args) 
            : m_func(std::forward<Callable>(f)) 
            , m_args(std::forward<Args>(args)...)
        {}

        void Run() 
        { 
            DoCall(std::index_sequence_for<Args...>{});
        }
    private:
        template<size_t... S>
            void DoCall(std::integer_sequence<size_t, S...>)
        {
            m_func(std::get<S>(m_args)...);
        }
    };
private:
    void StartThread(ImplPtr);

    template<typename Callable, typename... Args>
        std::unique_ptr<ThreadImpl<Callable, Args...> > MakeThreadImpl(Callable&& fn, Args&&... args)
        {
            return std::make_unique<ThreadImpl<Callable, Args...>>(std::forward<Callable>(fn), std::forward<Args>(args)...);
        }

    ////////////////////////////////////////
#ifdef USING_VS
    HANDLE m_thread;
#endif
#ifdef LINUX
    pthread_t m_thread;
#endif
};

template<class Fn, class... ArgType>
Thread::Thread(Fn&& f, ArgType&&... args)
{
    StartThread(MakeThreadImpl(std::forward<Fn>(f), std::forward<ArgType>(args)...));
}


////////////////////////////////////////////////////////////////////////////////
class Mutex
{
    friend class MutexLock;
public:
    Mutex();
    ~Mutex();

private:
    Mutex(const Mutex&) DELETED;
    Mutex& operator=(const Mutex&) DELETED;

#ifdef USING_VS
    CRITICAL_SECTION m_mutex;
#endif

#ifdef LINUX
    pthread_mutex_t m_mutex;
#endif
};

////////////////////////////////////////////////////////////////////////////////
class MutexLock
{
public:
    MutexLock(Mutex& m);
    ~MutexLock();
private:
    Mutex& m_mutex;

    MutexLock(const MutexLock&) DELETED;
    MutexLock& operator=(const MutexLock&) DELETED;
};

////////////////////////////////////////////////////////////////////////////////
int NumProcessors();
void YieldThread();

}

#endif
