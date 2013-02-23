#include <ctime>
#include <iostream>

#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "toolkit/common.hh"
#include "toolkit/timer.hh"

////////////////////////////////////////////////////////////////////////////////
// returns time in microseconds
static unsigned long long CurTime() 
{
#ifdef _WINDOWS
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	return (counter.QuadPart * 1000000) / freq.QuadPart;
#else
	struct timespec currentTime;
	clock_gettime(CLOCK_MONOTONIC, &currentTime);
	unsigned long long timeUsec = (unsigned long long)(currentTime.tv_sec * 1000000) +
		(unsigned long long)(currentTime.tv_nsec / 1000);
	return timeUsec;	
#endif
}

////////////////////////////////////////////////////////////////////////////////
Clock::Clock()
	: m_lastDt(0)
{
	m_lastTime = CurTime();
}
	
void Clock::Step(float minDt)
{
	unsigned long long cur = CurTime();
	unsigned long long diff = cur - m_lastTime;
	float dt = diff / 1e6f;
	if(dt < minDt)
	{
		m_lastDt = 0.0;
		return;
	}

	m_lastDt = dt;
	m_lastTime = cur;
}

////////////////////////////////////////////////////////////////////////////////
Timer::Timer()
	: m_startTime(0)
	, m_stopTime(0)
{
}

void Timer::Start()
{
	m_startTime = CurTime();
}

void Timer::Stop()
{
	m_stopTime = CurTime();
}
	
float Timer::GetTime()
{
	return (m_stopTime - m_startTime) / 1e6f;
}

////////////////////////////////////////////////////////////////////////////////	
unsigned long long SysTime::s_worldTime = 0;
float SysTime::s_dt = 0.f;

SysTime::SysTime(unsigned long long time)
	: m_time(time)
{}

SysTime SysTime::operator+(float timeInSeconds) const
{
	unsigned long long t = uint64_t(timeInSeconds * 1e6f);
	return m_time + t;
}

SysTime SysTime::Current()
{
	return s_worldTime;
}

SysTime SysTime::RealTime()
{
	return CurTime();
}

float SysTime::Dt() {
	return s_dt;
}

void SysTime::Advance(float delta)
{
	ASSERT(delta >= 0.f);
	long long t = int64_t(delta * 1e6f);
	s_worldTime += t;
	s_dt = delta;
}
	
float SysTime::operator-(const SysTime& o) const
{ 
	if(m_time > o.m_time)
		return (m_time - o.m_time) / 1e6f; 
	else 
		return ((o.m_time - m_time) / -1e6f);
}

