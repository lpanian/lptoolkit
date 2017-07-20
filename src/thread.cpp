#include <iostream>
#include "toolkit/thread.hh"

#include <cstring>

#ifdef LINUX
#include <unistd.h>
#include <sys/sysctl.h>
#endif

namespace lptk
{

Thread::Thread()
    : m_thread(0)
{
}
	
Thread::~Thread()
{
	if(joinable())
		std::terminate();
}

Thread::Thread(Thread&& o)
	: m_thread(0)
{
	swap(o);
}

Thread& Thread::operator=(Thread&& o)
{
	swap(o);
	return *this;
}

void Thread::swap(Thread& other)
{
	std::swap(m_thread, other.m_thread);
}

#ifdef USING_VS
static DWORD WINAPI ThreadMain(LPVOID lpParameter)
{
	Thread::ThreadImplBase* implBase = (Thread::ThreadImplBase*)lpParameter;
	implBase->Run();
	implBase->m_this.reset();
	return 0;
}
#endif

#ifdef LINUX
static void* ThreadMain(void* param)
{
	Thread::ThreadImplBase* implBase = (Thread::ThreadImplBase*)param;
	implBase->Run();
	implBase->m_this.reset();
	return 0;
}
#endif

void Thread::StartThread(ImplPtr impl)
{
    auto self = impl.get();
	self->m_this = std::move(impl); // create a ref to itself to keep the thread in memory until the end of ThreadMain

#ifdef USING_VS
	m_thread = CreateThread(
			NULL, // default security
			0, // default stack size
			ThreadMain, // func
			(LPVOID)self,
			0, // default creation flags
			NULL // thread id
			);
	if(m_thread == 0)
		self->m_this.reset();
#endif

#ifdef LINUX
	if(pthread_create(&m_thread, NULL, ThreadMain, (void*)self))
	{
		self->m_this.reset();
	}
#endif

}

bool Thread::joinable() 
{
	return m_thread != 0;
}

void Thread::join()
{
#ifdef USING_VS
	if(m_thread)
	{
		WaitForSingleObject(m_thread, INFINITE);
		CloseHandle(m_thread);
		m_thread = 0;
	}
#endif

#ifdef LINUX
	if(m_thread)
	{
		void* status;
		if(pthread_join(m_thread, &status))
			std::cerr << "Error joining thread" << std::endl;
		m_thread = 0;
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////
Mutex::Mutex()
{
#ifdef USING_VS
    InitializeCriticalSection(&m_mutex);
#endif

#ifdef LINUX
    int err;
    if((err = pthread_mutex_init(&m_mutex, NULL)) != 0) {
        std::cerr << "ERROR: failed to initialize pthread mutex: " << strerror(err) << std::endl;
    }
#endif
}

Mutex::~Mutex()
{
#ifdef USING_VS
    DeleteCriticalSection(&m_mutex);
#endif

#ifdef LINUX
    int err;
    if((err = pthread_mutex_destroy(&m_mutex)) != 0) {
        std::cerr << "ERROR: failed to destroy pthread mutex: " << strerror(err) << std::endl;
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
MutexLock::MutexLock(Mutex& m)
    : m_mutex(m)
{
#ifdef USING_VS
    EnterCriticalSection(&m_mutex.m_mutex);
#endif

#ifdef LINUX
    int err;
    if((err = pthread_mutex_lock(&m_mutex.m_mutex)) != 0) {
        std::cerr << "ERROR: failed to lock pthread mutex: " << strerror(err) << std::endl;
    }
#endif
}

MutexLock::~MutexLock()
{
#ifdef USING_VS
    LeaveCriticalSection(&m_mutex.m_mutex);
#endif

#ifdef LINUX
    int err;
    if((err = pthread_mutex_unlock(&m_mutex.m_mutex)) != 0) {
        std::cerr << "ERROR: failed to unlock pthread mutex: " << strerror(err) << std::endl;
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////
int NumProcessors()
{
    int count = 1;
#ifdef USING_VS
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    count = int(sysinfo.dwNumberOfProcessors);
#endif

#ifdef LINUX
    count = int(sysconf(_SC_NPROCESSORS_ONLN));
#endif
    return count;
}

void YieldThread()
{
#if defined(USING_VS)
    SwitchToThread();
#else if defined (LINUX)
    pthread_yield();
#endif
}

}

