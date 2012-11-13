#pragma once
#ifndef INCLUDED_toolkit_timer_HH
#define INCLUDED_toolkit_timer_HH

////////////////////////////////////////////////////////////////////////////////
// timer that works entirely with REAL time 
class Timer
{
public:
	Timer();

	void Start();
	void Stop();

	float GetTime();
private:
	unsigned long long m_startTime;
	unsigned long long m_stopTime;
};

////////////////////////////////////////////////////////////////////////////////
// used for SIMULATION time
class SysTime
{
public:
	SysTime(unsigned long long time = 0);

	static SysTime Current();		// current time
	static void Advance(float delta); // time to add to the current global time
	static float Dt();				// last time added to the clock
	static SysTime RealTime();		// current clock time, unaffected by time multiplier
	
	bool operator<(const SysTime& o) const { return m_time < o.m_time; }
	bool operator>(const SysTime& o) const { return m_time > o.m_time; }
	bool operator<=(const SysTime& o) const { return m_time <= o.m_time; }
	bool operator>=(const SysTime& o) const { return m_time >= o.m_time; }
	SysTime operator+(float timeInSeconds) const;
	SysTime operator+(const SysTime& o) const { return SysTime(m_time + o.m_time); }
	float operator-(const SysTime& o) const;

	unsigned long long Raw() const { return m_time; }
	
private:
	static float s_dt;
	static unsigned long long s_worldTime;	
	unsigned long long m_time;
};

#endif

