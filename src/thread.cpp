#include <iostream>
#include "toolkit/thread.hh"

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
	impl->m_this = impl; // create a ref to itself to keep the thread in memory until the end of ThreadMain
#ifdef USING_VS
	m_thread = CreateThread(
			NULL, // default security
			0, // default stack size
			ThreadMain, // func
			(LPVOID)impl.get(),
			0, // default creation flags
			NULL // thread id
			);
	if(m_thread == 0)
		impl->m_this.reset();
#endif

#ifdef LINUX
	if(pthread_create(&m_thread, NULL, ThreadMain, (void*)impl.get()))
	{
		impl->m_this.reset();
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
