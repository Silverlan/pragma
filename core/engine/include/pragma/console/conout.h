#ifndef __CONOUT_H__
#define __CONOUT_H__
#include "pragma/definitions.h"
#include <iostream>
#include <sstream>
#ifdef _WIN32
	#include <Windows.h>
#else
	#define DWORD unsigned int
	#define FOREGROUND_BLUE 1
	#define FOREGROUND_GREEN 2
	#define FOREGROUND_RED 4
	#define FOREGROUND_INTENSITY 8
	#define BACKGROUND_BLUE 16
	#define BACKGROUND_GREEN 32
	#define BACKGROUND_RED 64
	#define BACKGROUND_INTENSITY 128
#endif

namespace Con {class c_crit;};
template <class T> Con::c_crit& operator<< (Con::c_crit &con,const T &t);
namespace Con
{
	class DLLENGINE c_cout {};
	class DLLENGINE c_cwar {};
	class DLLENGINE c_cerr {};
	class DLLENGINE c_crit
	{
	private:
		std::stringstream m_message;
		bool m_bActivated = false;
	public:
		friend DLLENGINE std::basic_ostream<char,std::char_traits<char>> &endl(std::basic_ostream<char,std::char_traits<char>>& os);
		template <class T>
			friend Con::c_crit& ::operator<<(Con::c_crit &con,const T &t);
	};
	class DLLENGINE c_csv {};
	class DLLENGINE c_ccl {};
	extern DLLENGINE c_cout cout;
	extern DLLENGINE c_cwar cwar;
	extern DLLENGINE c_cerr cerr;
	extern DLLENGINE c_crit crit;
	extern DLLENGINE c_csv csv;
	extern DLLENGINE c_ccl ccl;
	DLLENGINE std::basic_ostream<char,std::char_traits<char>> &endl(std::basic_ostream<char,std::char_traits<char>>& os);
	DLLENGINE void flush();
	DLLENGINE void attr(DWORD attr);
	DLLENGINE void WriteToLog(std::stringstream &ss);
	DLLENGINE void WriteToLog(std::string str);
	DLLENGINE int GetLogLevel();
};

// c_cout
template <class T> Con::c_cout& operator<<(Con::c_cout &con,const T &t)
{
	std::cout<<t;
	if(Con::GetLogLevel() >= 3)
	{
		std::stringstream ss;
		ss<<t;
		Con::WriteToLog(ss);
	}
	return con;
}
typedef std::ostream& (*conmanipulator) (std::ostream&);
DLLENGINE Con::c_cout& operator<<(Con::c_cout& con,conmanipulator manipulator);
//

// c_cwar
template <class T> Con::c_cwar& operator<< (Con::c_cwar &con,const T &t)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut,FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	std::cout<<t;
#else
	std::cout<<"\033[33;1m"<<t;
#endif
	if(Con::GetLogLevel() >= 2)
	{
		std::stringstream ss;
		ss<<t;
		Con::WriteToLog(ss);
	}
	return con;
}
typedef std::ostream& (*conmanipulator) (std::ostream&);
DLLENGINE Con::c_cwar& operator<<(Con::c_cwar &con,conmanipulator manipulator);
//

// c_cerr
template <class T> Con::c_cerr& operator<< (Con::c_cerr &con,const T &t)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut,FOREGROUND_RED | FOREGROUND_INTENSITY);
	std::cout<<t;
#else
	std::cout<<"\033[31;1m"<<t;
#endif
	if(Con::GetLogLevel() >= 1)
	{
		std::stringstream ss;
		ss<<t;
		Con::WriteToLog(ss);
	}
	return con;
}
typedef std::ostream& (*conmanipulator) (std::ostream&);
DLLENGINE Con::c_cerr& operator<<(Con::c_cerr &con,conmanipulator manipulator);
//

// c_crit
template <class T> Con::c_crit& operator<< (Con::c_crit &con,const T &t)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut,BACKGROUND_RED | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	std::cout<<t;
#else
	std::cout<<"\033[41;37;1m"<<t;
#endif
	std::stringstream ss;
	ss<<t;
	if(Con::GetLogLevel() >= 1)
		Con::WriteToLog(ss);
	Con::crit.m_bActivated = true;
	Con::crit.m_message<<t;
	return con;
}
typedef std::ostream& (*conmanipulator) (std::ostream&);
DLLENGINE Con::c_crit& operator<<(Con::c_crit &con,conmanipulator manipulator);
//

// c_csv
template <class T> Con::c_csv& operator<< (Con::c_csv &con,const T &t)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut,FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	std::cout<<t;
#else
	std::cout<<"\033[36;1m"<<t;
#endif
	if(Con::GetLogLevel() >= 2)
	{
		std::stringstream ss;
		ss<<t;
		Con::WriteToLog(ss);
	}
	return con;
}
typedef std::ostream& (*conmanipulator) (std::ostream&);
DLLENGINE Con::c_csv& operator<<(Con::c_csv &con,conmanipulator manipulator);
//

// c_ccl
template <class T> Con::c_ccl& operator<< (Con::c_ccl &con,const T &t)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut,FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	std::cout<<t;
#else
	std::cout<<"\033[35;1m"<<t;
#endif
	if(Con::GetLogLevel() >= 2)
	{
		std::stringstream ss;
		ss<<t;
		Con::WriteToLog(ss);
	}
	return con;
}
typedef std::ostream& (*conmanipulator) (std::ostream&);
DLLENGINE Con::c_ccl& operator<<(Con::c_ccl &con,conmanipulator manipulator);
//

#endif