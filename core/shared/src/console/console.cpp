/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/console/console.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
 
DebugConsole::DebugConsole() :
	_cinbuf(0), _coutbuf(0), _cerrbuf(0)
{
}
 
DebugConsole::~DebugConsole() {}

void DebugConsole::open()
{
#ifdef _WIN32
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	this->_cinbuf = std::cin.rdbuf();
	this->_console_cin.open("CONIN$");
	std::cin.rdbuf(this->_console_cin.rdbuf());
	this->_coutbuf = std::cout.rdbuf();
	this->_console_cout.open("CONOUT$");
	std::cout.rdbuf(this->_console_cout.rdbuf());
	this->_cerrbuf = std::cerr.rdbuf();
	this->_console_cerr.open("CONOUT$");
	std::cerr.rdbuf(this->_console_cerr.rdbuf());
#endif
}
 
void DebugConsole::close()
{
#ifdef _WIN32
	this->_console_cout.close();
	std::cout.rdbuf(this->_coutbuf);
	//this->_console_cin.close(); // This used to work until windows 7, now it blocks the process until new input is received
	//std::cin.rdbuf(this->_cinbuf);
	this->_console_cerr.close();
	std::cerr.rdbuf(this->_cerrbuf);
	INPUT_RECORD input;
	unsigned long numEvents;
	WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE),&input,0,&numEvents); // Workaround: Writes to the console to make sure the thread can end properly
	//CloseHandle(GetStdHandle(STD_INPUT_HANDLE)); // Doesn't work?
	FreeConsole();
#endif
}