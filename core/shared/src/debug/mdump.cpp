/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#ifdef _WIN32
#include "pragma/debug/mdump.h"
#include <tchar.h>
#include <fsys/filesystem.h>
#include <util_zip.h>
#include <sharedutils/util_debug.h>
#include <sharedutils/util_clock.hpp>
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include "pragma/engine_info.hpp"
#include "pragma/logging.hpp"

extern DLLNETWORK Engine *engine;

LPCSTR MiniDumper::m_szAppName;

std::string g_crashExceptionMessage = {};
MiniDumper::MiniDumper(LPCSTR szAppName)
{
	// if this assert fires then you have two instances of MiniDumper
	// which is not allowed
	assert(m_szAppName == NULL);

	m_szAppName = szAppName ? strdup(szAppName) : "Application";

	::SetUnhandledExceptionFilter(TopLevelFilter);
	// Note: set_terminate handler is called before SetUnhandledExceptionFilter.
	// set_terminate allows us to retrieve the underlying message from the exception (if there was one)
	set_terminate([]() {
		auto eptr = std::current_exception();
		if(!eptr) {
			g_crashExceptionMessage = {};
			return;
		}
		try {
			std::rethrow_exception(eptr);
		}
		catch(const std::exception &e) {
			g_crashExceptionMessage = std::string {typeid(e).name()} + ": " + e.what();
		}
		catch(...) {
			g_crashExceptionMessage = "Unknown Exception";
		}
		// Relay exception to SetUnhandledExceptionFilter
		std::rethrow_exception(eptr);
	});
}

LONG MiniDumper::TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	// MessageBox(0,s_exceptionMessage.c_str(),"Exception",MB_OK);
	LONG retval = EXCEPTION_CONTINUE_SEARCH;
	HWND hParent = NULL; // find a better value for your app

	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old
	// (e.g. Windows 2000)
	HMODULE hDll = NULL;
	char szDbgHelpPath[_MAX_PATH];

	if(GetModuleFileName(NULL, szDbgHelpPath, _MAX_PATH)) {
		char *pSlash = _tcsrchr(szDbgHelpPath, '\\');
		if(pSlash) {
			_tcscpy(pSlash + 1, "DBGHELP.DLL");
			hDll = ::LoadLibrary(szDbgHelpPath);
		}
	}

	if(hDll == NULL) {
		// load any version we can
		hDll = ::LoadLibrary("DBGHELP.DLL");
	}

	spdlog::info("Generating crashdump...");
	pragma::flush_loggers();

	LPCTSTR szResult = NULL;

	auto shouldShowMsBox = (util::get_subsystem() == util::SubSystem::GUI && !engine->IsNonInteractiveMode());
	if(hDll) {
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
		if(pDump) {
			FileManager::CreateDirectory("crashdumps");
			auto programPath = util::get_program_path();
			auto szDumpPath = programPath + "/crashdumps/";
			szDumpPath += std::string(m_szAppName) + std::string(".dmp");
			char szScratch[_MAX_PATH];

			// ask the user if they want to save a dump file
			if(!shouldShowMsBox
			  || (::MessageBox(NULL,
			        "A terminal error has occurred in the program. Would you like to save a diagnostic file? This file contains information about your system and the state of the game at the time of the crash and can be utilized by a developer to fix the underlying problem.", m_szAppName,
			        MB_YESNO)
			    == IDYES)) {
				// create the file
				HANDLE hFile = ::CreateFileA(szDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if(hFile != INVALID_HANDLE_VALUE) {
					_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;

					// write the dump
					//MiniDumpWithFullMemory
					BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
					::CloseHandle(hFile);
					if(bOK) {
						std::string err;
						std::string zipFileName;
						pragma::detail::close_logger();
						auto zipFile = Engine::GenerateEngineDump("crashdumps/crashdump", zipFileName, err);
						if(zipFile) {
							// Write Minidump
							VFilePtrReal f = nullptr;
							auto t = util::Clock::now();
							while(f == nullptr) // Wait until dump has been written
							{
								auto tNow = util::Clock::now();
								auto tDelta = std::chrono::duration_cast<std::chrono::seconds>(tNow - t).count();
								if(tDelta >= 4) // Don't wait more than 4 seconds
									break;
								std::this_thread::sleep_for(std::chrono::milliseconds(250));
								f = FileManager::OpenSystemFile(szDumpPath.c_str(), "rb");
							}
							if(f != nullptr) {
								auto size = f->GetSize();
								std::vector<uint8_t> dumpData(size);
								f->Read(dumpData.data(), size);
								f = nullptr;
								zipFile->AddFile(ufile::get_file_from_filename(szDumpPath), dumpData.data(), size);
								std::remove(szDumpPath.c_str());
							}
							zipFile = nullptr;

							sprintf(szScratch, "Saved dump file to '%s'. Please send it to a developer, along with a description of what you did to trigger the error.", zipFileName.c_str() /*,engine_info::get_author_mail_address().c_str()*/);
							util::open_path_in_explorer(ufile::get_path_from_filename(zipFileName), ufile::get_file_from_filename(zipFileName));
							szResult = szScratch;
							retval = EXCEPTION_EXECUTE_HANDLER;
						}
						else {
							sprintf(szScratch, err.c_str());
							szResult = szScratch;
						}
					}
					else {
						sprintf(szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath.c_str(), GetLastError());
						szResult = szScratch;
					}
				}
				else {
					sprintf(szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath.c_str(), GetLastError());
					szResult = szScratch;
				}
			}
		}
		else {
			szResult = "DBGHELP.DLL too old";
		}
	}
	else {
		szResult = "DBGHELP.DLL not found";
	}

	if(szResult && shouldShowMsBox)
		::MessageBox(NULL, szResult, m_szAppName, MB_OK);

	return retval;
}

#endif
