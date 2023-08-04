/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/console/console.h"
#include "pragma/logging.hpp"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

DebugConsole::DebugConsole() : _cinbuf(0), _coutbuf(0), _cerrbuf(0) {}

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

	freopen("CON", "w", stdout); // Redirect printf, etc.

	// Enable ANSI color codes under Windows
	HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if(handleOut) {
		DWORD consoleMode;
		if(GetConsoleMode(handleOut, &consoleMode)) {
			consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
			consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;
			SetConsoleMode(handleOut, consoleMode);
		}
	}
	//

	// Change the console font
	if(handleOut) {
		auto fontPath = util::get_program_path() + "\\fonts\\ubuntu\\UbuntuMono-R.ttf";
		ustring::replace(fontPath, "/", "\\");
		HANDLE m_stdOut = handleOut;
		auto numFontsAdded = AddFontResourceEx(fontPath.c_str(), FR_NOT_ENUM, 0);
		if(numFontsAdded > 0) {
			SendNotifyMessage(HWND_BROADCAST, WM_FONTCHANGE, 0, 0);

			CONSOLE_FONT_INFOEX cfi;
			cfi.cbSize = sizeof cfi;
			cfi.nFont = 0;
			cfi.dwFontSize.X = 0;
			cfi.dwFontSize.Y = 18;
			cfi.FontFamily = FF_DONTCARE;
			cfi.FontWeight = FW_NORMAL;
			wcscpy(cfi.FaceName, L"Ubuntu Mono");
			auto res = SetCurrentConsoleFontEx(handleOut, FALSE, &cfi);
			if(res == 0) {
				auto errMsg = util::get_last_system_error_string();
				spdlog::warn("Failed to set console font: {}", errMsg);
			}
		}
		else
			spdlog::warn("Failed to set console font: AddFontResourceEx failed.");
	}

#else
	// this will barf out everything.
	this->_cinbuf = std::cin.rdbuf();
	this->_coutbuf = std::cout.rdbuf();
	this->_cerrbuf = std::cerr.rdbuf();
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
	WriteConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &input, 0, &numEvents); // Workaround: Writes to the console to make sure the thread can end properly
	//CloseHandle(GetStdHandle(STD_INPUT_HANDLE)); // Doesn't work?
	fclose(stdin);
	fclose(stdout);
	fclose(stderr);
	FreeConsole();
#else
	std::cout.rdbuf(this->_coutbuf);
	std::cerr.rdbuf(this->_cerrbuf);
#endif
}
