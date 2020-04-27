/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_engine.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include "pragma/console/cvar.h"
#include <pragma/console/convars.h>
#include <sharedutils/util_debug.h>
#include <mathutil/color.h>

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

static std::function<void(const std::string_view&,Con::MessageFlags,const Color*)> s_outputCallback = nullptr;
void Con::set_output_callback(const std::function<void(const std::string_view&,MessageFlags,const ::Color*)> &callback) {s_outputCallback = callback;}
const std::function<void(const std::string_view&,Con::MessageFlags,const Color*)> &Con::get_output_callback() {return s_outputCallback;}
void Con::print(const std::string_view &sv,const ::Color &color,MessageFlags flags)
{
	util::set_console_color(util::color_to_console_color_flags(color));
	std::cout<<sv;
	Con::flush();
	auto &outputCallback = Con::get_output_callback();
	if(outputCallback == nullptr)
		return;
	outputCallback(sv,flags,&color);
}
void Con::print(const std::string_view &sv,MessageFlags flags)
{
	std::cout<<sv;
	Con::flush();
	auto &outputCallback = Con::get_output_callback();
	if(outputCallback == nullptr)
		return;
	outputCallback(sv,flags,nullptr);
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
	invoke_output_callback(manipulator,Con::MessageFlags::Generic);
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
	invoke_output_callback(manipulator,Con::MessageFlags::Warning);
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
	invoke_output_callback(manipulator,Con::MessageFlags::Error);
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
	invoke_output_callback(manipulator,Con::MessageFlags::Critical);
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
	invoke_output_callback(manipulator,Con::MessageFlags::ServerSide);
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
	invoke_output_callback(manipulator,Con::MessageFlags::ClientSide);
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
	//os.flush();
	//flush();
#else
	os<<"\n";
	//os<<"\033[0m"<<"\n";
#endif
	util::reset_console_color();
	if(bCrit == true)
		std::this_thread::sleep_for(std::chrono::seconds(5));
	return os;
}
