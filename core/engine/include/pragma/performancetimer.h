#ifndef __PERFORMANCETIMER_H__
#define __PERFORMANCETIMER_H__

#ifdef _DEBUG
#define ENABLE_PERFORMANCE_TIMER 1
#endif
#define ENABLE_PERFORMANCE_TIMER 1

#ifdef ENABLE_PERFORMANCE_TIMER
#include "pragma/definitions.h"
#include <chrono>
#include <vector>
#include <string>

class PerformanceTimer;
class DLLENGINE PerformanceTimerInstance
{
public:
	friend PerformanceTimer;
protected:
	PerformanceTimerInstance(std::string identifier);
	std::string m_identifier;
	std::chrono::time_point<std::chrono::system_clock> m_tStart;
	long double m_sum;
	long double m_last;
	std::chrono::time_point<std::chrono::system_clock> m_tLastReset;
	unsigned long long m_count;
	unsigned long long m_errorCount;
	void StartMeasurement();
	void EndMeasurement();
	long double GetAverage();
	void Reset();
};

class DLLENGINE PerformanceTimer
{
private:
	static std::vector<PerformanceTimerInstance> m_timers;
	static void GetSortedTimers(std::vector<PerformanceTimerInstance*> &timers);
public:
	static unsigned int InitializeTimer(std::string identifier);
	static void StartMeasurement(unsigned int id);
	static void EndMeasurement(unsigned int id);
	static long double GetResult(unsigned int id);
	static void SaveResults();
	static void PrintResults();
	static void Reset();
	static void Reset(unsigned int id);
};
#endif

#endif