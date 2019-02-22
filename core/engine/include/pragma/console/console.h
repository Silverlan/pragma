#ifndef __DBGCONSOLE_H__
#define __DBGCONSOLE_H__
#include "pragma/definitions.h"
#include <streambuf>
#include <fstream>
 
class DLLENGINE DebugConsole
{
public:
	DebugConsole();
	~DebugConsole();
 
	void open();
	void close();
 
private:
	std::streambuf	*_cinbuf;
	std::streambuf	*_coutbuf;
	std::streambuf	*_cerrbuf;
	std::ifstream	_console_cin;
	std::ofstream	_console_cout;
	std::ofstream	_console_cerr;
};
 
#endif // __DBGCONSOLE_H__