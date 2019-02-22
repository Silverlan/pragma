#include "stdafx_engine.h"
#include "pragma/performancetimer.h"
#ifdef ENABLE_PERFORMANCE_TIMER
#include "pragma/console/conout.h"
#include <fsys/filesystem.h>

std::vector<PerformanceTimerInstance> PerformanceTimer::m_timers;

unsigned int PerformanceTimer::InitializeTimer(std::string identifier)
{
	if(!m_timers.empty())
	{
		for(auto i=0;i<m_timers.size();i++)
		{
			if(m_timers[i].m_identifier == identifier)
				return i;
		}
	}
	m_timers.push_back(PerformanceTimerInstance(identifier));
	return static_cast<unsigned int>(m_timers.size()) -1;
}

void PerformanceTimer::StartMeasurement(unsigned int id)
{
	PerformanceTimerInstance &timer = m_timers[id];
	timer.StartMeasurement();
}
void PerformanceTimer::EndMeasurement(unsigned int id)
{
	PerformanceTimerInstance &timer = m_timers[id];
	timer.EndMeasurement();
}
long double PerformanceTimer::GetResult(unsigned int id)
{
	PerformanceTimerInstance &timer = m_timers[id];
	return timer.GetAverage();
}
void PerformanceTimer::Reset(unsigned int id)
{
	PerformanceTimerInstance &timer = m_timers[id];
	return timer.Reset();
}
void PerformanceTimer::Reset()
{
	for(unsigned int i=0;i<m_timers.size();i++)
	{
		PerformanceTimerInstance &timer = m_timers[i];
		timer.Reset();
	}
}
void PerformanceTimer::SaveResults()
{
#ifdef _DEBUG
	const char *fname = "performance_debug.txt";
#else
	const char *fname = "performance.txt";
#endif
	auto f = FileManager::OpenFile<VFilePtrReal>(fname,"w");
	if(f == NULL)
		return;
	std::vector<PerformanceTimerInstance*> timersSorted;
	GetSortedTimers(timersSorted);
	for(unsigned int i=0;i<timersSorted.size();i++)
	{
		PerformanceTimerInstance &timer = *timersSorted[i];
		std::chrono::time_point<std::chrono::system_clock> tCur = std::chrono::system_clock::now();
		std::chrono::duration<double> duration = tCur -timer.m_tLastReset;
		double elapsed = duration.count();// *std::chrono::milliseconds::period::num /std::chrono::milliseconds::period::den;
		std::stringstream out;
		out<<"\tAverage: "<<std::fixed<<timer.GetAverage()<<std::scientific<<" ("<<timer.GetAverage()<<")"<<"\n";
		out<<"\tLast: "<<std::fixed<<timer.m_last<<std::scientific<<" ("<<timer.m_last<<")"<<"\n";
		out<<"\tCall Count: "<<timer.m_count<<"\n";
		out<<"\tTotal time passed: "<<std::fixed<<elapsed<<std::scientific<<" ("<<elapsed<<")"<<"\n";
		out<<"\tCalls per second: "<<std::fixed<<(timer.m_count /elapsed)<<"\n";
		f->WriteString(out.str());
	}
}
void PerformanceTimer::PrintResults()
{
	Con::cout<<"---------- PERFORMANCE RESULTS START ----------"<<Con::endl;
	std::vector<PerformanceTimerInstance*> timersSorted;
	GetSortedTimers(timersSorted);
	for(unsigned int i=0;i<timersSorted.size();i++)
	{
		PerformanceTimerInstance &timer = *timersSorted[i];
		std::chrono::time_point<std::chrono::system_clock> tCur = std::chrono::system_clock::now();
		std::chrono::duration<double> duration = tCur -timer.m_tLastReset;
		double elapsed = duration.count();// *std::chrono::milliseconds::period::num /std::chrono::milliseconds::period::den;
		Con::cout<<timer.m_identifier<<Con::endl;
		Con::cout<<"\tAverage: "<<std::fixed<<timer.GetAverage()<<std::scientific<<" ("<<timer.GetAverage()<<")"<<Con::endl;
		Con::cout<<"\tLast: "<<std::fixed<<timer.m_last<<std::scientific<<" ("<<timer.m_last<<")"<<Con::endl;
		Con::cout<<"\tCall Count: "<<timer.m_count<<Con::endl;
		Con::cout<<"\tTotal time passed: "<<std::fixed<<elapsed<<std::scientific<<" ("<<elapsed<<")"<<Con::endl;
		Con::cout<<"\tCalls per second: "<<std::fixed<<(timer.m_count /elapsed)<<Con::endl;
	}
	Con::cout<<"----------- PERFORMANCE RESULTS END -----------"<<Con::endl;
}
void PerformanceTimer::GetSortedTimers(std::vector<PerformanceTimerInstance*> &timers)
{
	for(unsigned int i=0;i<m_timers.size();i++)
	{
		PerformanceTimerInstance &timer = m_timers[i];
		long double avg = timer.GetAverage();
		bool bInserted = false;
		for(unsigned int j=0;j<timers.size();j++)
		{
			PerformanceTimerInstance *tOther = timers[j];
			long double avgOther = tOther->GetAverage();
			if(avg >= avgOther)
			{
				timers.insert(timers.begin() +j,&timer);
				bInserted = true;
				break;
			}
		}
		if(bInserted == false)
			timers.push_back(&timer);
	}
}

////////////////////////////////////

PerformanceTimerInstance::PerformanceTimerInstance(std::string identifier)
	: m_identifier(identifier)
{
	Reset();
}

void PerformanceTimerInstance::StartMeasurement()
{
	m_tStart = std::chrono::system_clock::now();
}

void PerformanceTimerInstance::EndMeasurement()
{
	double errorMargin = 10.0;
	long double avg = GetAverage();
	long double avgError = avg;
	avgError *= (errorMargin +1.0);
	avgError -= avg;
	std::chrono::time_point<std::chrono::system_clock> tEnd = std::chrono::system_clock::now();
	std::chrono::duration<double> duration = tEnd -m_tStart;
	double elapsed = duration.count();// *std::chrono::milliseconds::period::num /std::chrono::milliseconds::period::den;
	m_last = elapsed;
	if(abs(elapsed -avg) <= avgError || m_errorCount > 10)
	{
		m_sum += elapsed;
		m_count++;
		if(m_errorCount > 10)
			m_errorCount--;
	}
	else
	{
		m_errorCount++;
		if(m_errorCount > 10)
		{
			//Con::cwar<<"WARNING: PerformanceTimer '"<<m_identifier<<"' out of error margin!"<<Con::endl;
			m_errorCount = 20;
		}
	}
}

long double PerformanceTimerInstance::GetAverage()
{
	if(m_count == 0)
		return 0.0;
	return m_sum /m_count;
}

void PerformanceTimerInstance::Reset()
{
	m_sum = 0;
	m_count = 0;
	m_last = 0;
	m_errorCount = 0;
	m_tLastReset = std::chrono::system_clock::now();
}
#endif