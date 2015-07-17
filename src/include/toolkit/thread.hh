#pragma once
#ifndef INCLUDED_toolkit_thread_hh
#define INCLUDED_toolkit_thread_hh

#include <functional>
#include <memory>
#if defined(LINUX)
#include <pthread.h>
#endif

namespace lptk
{

////////////////////////////////////////////////////////////////////////////////
// minimal replacement for <thread> - <thread> on windows requires 
// c++ exceptions. this makes me sad.
class Thread
{
public:
    struct ThreadImplBase;
    typedef std::shared_ptr<ThreadImplBase> ImplPtr;

    Thread(); // doesn't do anything, just empty
    Thread(Thread&&); // move
    Thread& operator=(Thread&&);

    template<class Fn, class... ArgType>
        explicit Thread(Fn&& f, ArgType&&... args);
    ~Thread();

    void swap(Thread& other);
    bool joinable() ;
    void join();

    ////////////////////////////////////////	
    struct ThreadImplBase
    {
        std::shared_ptr<ThreadImplBase> m_this;
        inline virtual ~ThreadImplBase() {}
        virtual void Run() = 0;
    };

    template<typename Callable>
        struct ThreadImpl : public ThreadImplBase
    {
        Callable m_func;
        ThreadImpl(Callable&& f) : m_func(std::forward<Callable>(f)) {}
        void Run() { m_func(); }
    };
private:
    Thread(const Thread&) DELETED; // no copy
    Thread& operator=(const Thread&) DELETED; // no copy

    void StartThread(ImplPtr);

    template<class Callable>
        std::shared_ptr<ThreadImpl<Callable> > MakeThreadImpl(Callable&& fn)
        {
            return std::make_shared<ThreadImpl<Callable>>(std::forward<Callable>(fn));
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
    StartThread(MakeThreadImpl(std::bind<void>(std::forward<Fn>(f), 
                    std::forward<ArgType>(args)...)));
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

}

#endif
