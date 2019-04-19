#include "stdafx_engine.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include "pragma/console/cvar.h"
#include <pragma/console/convars.h>
#include <sharedutils/util_debug.h>

DLLENGINE Con::c_cout Con::cout;
DLLENGINE Con::c_cwar Con::cwar;
DLLENGINE Con::c_cerr Con::cerr;
DLLENGINE Con::c_crit Con::crit;
DLLENGINE Con::c_csv Con::csv;
DLLENGINE Con::c_ccl Con::ccl;

extern DLLENGINE Engine *engine;
static CVar cvLog = GetConVar("log_enabled");
int Con::GetLogLevel()
{
	if(engine == NULL || !engine->IsRunning())
		return 0;
	return cvLog->GetInt();
}

void Con::WriteToLog(std::stringstream &ss)
{
	if(engine == NULL)
		return;
	WriteToLog(ss.str());
}

void Con::WriteToLog(std::string str)
{
	if(engine == NULL)
		return;
	engine->WriteToLog(str);
}

////////////////////////////////

void Con::attr(DWORD attr)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut,static_cast<WORD>(attr));
#endif
}

void Con::flush()
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut,FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

Con::c_cout& operator<<(Con::c_cout& con,conmanipulator manipulator)
{
	std::cout<<manipulator;
	if(Con::GetLogLevel() >= 3)
	{
		std::stringstream ss;
		ss<<manipulator;
		Con::WriteToLog(ss);
	}
	return con;
}

Con::c_cwar& operator<<(Con::c_cwar &con,conmanipulator manipulator)
{
	std::cout<<manipulator;
	if(Con::GetLogLevel() >= 2)
	{
		std::stringstream ss;
		ss<<manipulator;
		Con::WriteToLog(ss);
	}
	return con;
}

Con::c_cerr& operator<<(Con::c_cerr &con,conmanipulator manipulator)
{
	std::cout<<manipulator;
	if(Con::GetLogLevel() >= 1)
	{
		std::stringstream ss;
		ss<<manipulator;
		Con::WriteToLog(ss);
	}
	return con;
}

Con::c_crit& operator<<(Con::c_crit &con,conmanipulator manipulator)
{
	std::cout<<manipulator;
	if(Con::GetLogLevel() >= 1)
	{
		std::stringstream ss;
		ss<<manipulator;
		Con::WriteToLog(ss);
	}
	return con;
}

Con::c_csv& operator<<(Con::c_csv &con,conmanipulator manipulator)
{
	std::cout<<manipulator;
	if(Con::GetLogLevel() >= 2)
	{
		std::stringstream ss;
		ss<<manipulator;
		Con::WriteToLog(ss);
	}
	return con;
}

Con::c_ccl& operator<<(Con::c_ccl &con,conmanipulator manipulator)
{
	std::cout<<manipulator;
	if(Con::GetLogLevel() >= 2)
	{
		std::stringstream ss;
		ss<<manipulator;
		Con::WriteToLog(ss);
	}
	return con;
}

std::basic_ostream<char,std::char_traits<char>> &Con::endl(std::basic_ostream<char,std::char_traits<char>>& os)
{
	auto &bCrit = Con::crit.m_bActivated;
	if(bCrit == true)
	{
		bCrit = false;
#ifdef _WIN32
		//UTIL_ASSERT(false,Con::crit.m_message.str(),false);
#endif
		Con::crit.m_message.clear();
	}

#ifdef _WIN32
	os.put('\n');
	os.flush();
	flush();
#else
	os<<"\033[0m"<<"\n";
#endif
	if(bCrit == true)
		std::this_thread::sleep_for(std::chrono::seconds(5));
	return os;
}
