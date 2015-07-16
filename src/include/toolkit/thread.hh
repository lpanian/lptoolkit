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

//#ifndef USING_VS
    template<class Fn, class... ArgType>
        explicit Thread(Fn&& f, ArgType&&... args);
//#else
//    template<class Fn>
//        explicit Thread(Fn&& f);
//    template<class Fn, class A0>
//        explicit Thread(Fn&& f, A0&& a0);
//    template<class Fn, class A0, class A1>
//        explicit Thread(Fn&& f, A0&& a0, A1&& a1);
//    template<class Fn, class A0, class A1, class A2>
//        explicit Thread(Fn&& f, A0&& a0, A1&& a1, A2&& a2);
//    template<class Fn, class A0, class A1, class A2, class A3>
//        explicit Thread(Fn&& f, A0&& a0, A1&& a1, A2&& a2,  A3&& a3);
//    template<class Fn, class A0, class A1, class A2, class A3, class A4>
//        explicit Thread(Fn&& f, A0&& a0, A1&& a1, A2&& a2,  A3&& a3, A4&& a4);

//#endif
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

//#ifndef USING_VS
template<class Fn, class... ArgType>
Thread::Thread(Fn&& f, ArgType&&... args)
{
    StartThread(MakeThreadImpl(std::bind<void>(std::forward<Fn>(f), 
                    std::forward<ArgType>(args)...)));
}

//#else
//    template<class Fn>
//Thread::Thread(Fn&& f)
//{
//    StartThread( MakeThreadImpl(std::bind<void>(
//                    std::forward<Fn>(f)
//                    )));
//}

//    template<class Fn, class A0>
//Thread::Thread(Fn&& f, A0&& a0)
//{
//    StartThread(MakeThreadImpl(std::bind<void>(
//                    std::forward<Fn>(f), 
//                    std::forward<A0>(a0) 
//                    )));
//}

//    template<class Fn, class A0, class A1>
//Thread::Thread(Fn&& f, A0&& a0, A1&& a1)
//{
//    StartThread(MakeThreadImpl(std::bind<void>(
//                    std::forward<Fn>(f), 
//                    std::forward<A0>(a0),
//                    std::forward<A1>(a1) 
//                    )));
//}

//    template<class Fn, class A0, class A1, class A2>
//Thread::Thread(Fn&& f, A0&& a0, A1&& a1, A2&& a2)
//{
//    StartThread(MakeThreadImpl(std::bind<void>(
//                    std::forward<Fn>(f), 
//                    std::forward<A0>(a0),
//                    std::forward<A1>(a1),
//                    std::forward<A2>(a2) 
//                    )));
//}

//    template<class Fn, class A0, class A1, class A2, class A3>
//Thread::Thread(Fn&& f, A0&& a0, A1&& a1, A2&& a2,  A3&& a3)
//{
//    StartThread(MakeThreadImpl(std::bind<void>(
//                    std::forward<Fn>(f), 
//                    std::forward<A0>(a0),
//                    std::forward<A1>(a1),
//                    std::forward<A2>(a2),
//                    std::forward<A3>(a3) 
//                    )));
//}

//    template<class Fn, class A0, class A1, class A2, class A3, class A4>
//Thread::Thread(Fn&& f, A0&& a0, A1&& a1, A2&& a2,  A3&& a3, A4&& a4)
//{
//    StartThread(MakeThreadImpl(std::bind<void>(
//                    std::forward<Fn>(f), 
//                    std::forward<A0>(a0),
//                    std::forward<A1>(a1),
//                    std::forward<A2>(a2),
//                    std::forward<A3>(a3),
//                    std::forward<A4>(a4) 
//                    )));
//}

//#endif

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
