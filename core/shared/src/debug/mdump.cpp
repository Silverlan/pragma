/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/debug/mdump.h"
#include "pragma/debug/debug_utils.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_debug.h>
#include <sharedutils/util_clock.hpp>
#include <sharedutils/util.h>
#include <sharedutils/util_file.h>
#include <exception>
#include "pragma/engine_info.hpp"
#include "pragma/logging.hpp"
#include "pragma/localization.h"
#ifdef _WIN32
#include <tchar.h>
#else
#include <signal.h>
#include <execinfo.h>
#endif

import util_zip;

extern DLLNETWORK Engine *engine;

using namespace pragma::debug;

std::string g_crashExceptionMessage = {};
static CrashHandler *g_crashHandler = nullptr;
CrashHandler::CrashHandler(const std::string &appName) : m_appName {appName}
{
	assert(!g_crashHandler);
	g_crashHandler = this;
#ifdef _WIN32
	::SetUnhandledExceptionFilter(TopLevelFilter);
#else
	signal(SIGSEGV, +[](int sig) {
		if(!g_crashHandler) {
			exit(1);
			return;
		}
		g_crashHandler->m_sig = sig;
		g_crashHandler->GenerateCrashDump();
		exit(1);
	});
#endif
	// Note: set_terminate handler is called before SetUnhandledExceptionFilter.
	// set_terminate allows us to retrieve the underlying message from the exception (if there was one)
	std::set_terminate([]() {
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

CrashHandler::~CrashHandler() { g_crashHandler = nullptr; }

#ifdef _WIN32
std::optional<std::string> CrashHandler::GenerateMiniDump(std::string &outErr) const
{
	if(!m_pExceptionInfo) {
		outErr = "No exception info!";
		return {};
	}
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

	if(!hDll) {
		outErr = "DBGHELP.DLL not found";
		return {};
	}

	auto programPath = util::get_program_path();
	auto szDumpPath = programPath + "/crashdumps/";
	szDumpPath += m_appName + std::string(".dmp");

	MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
	if(!pDump) {
		outErr = "DBGHELP.DLL too old";
		return {};
	}

	// create the file
	HANDLE hFile = ::CreateFileA(szDumpPath.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFile == INVALID_HANDLE_VALUE) {
		outErr = "Failed to open file '" + szDumpPath + "' for writing!";
		return {};
	}
	_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

	ExInfo.ThreadId = ::GetCurrentThreadId();
	ExInfo.ExceptionPointers = m_pExceptionInfo;
	ExInfo.ClientPointers = NULL;

	// write the dump
	//MiniDumpWithFullMemory
	BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
	::CloseHandle(hFile);
	if(bOK)
		return szDumpPath;
	outErr = "Failed to write minidump!";
	return {};
}
#endif

bool CrashHandler::GenerateCrashDump() const
{
	spdlog::info("Generating crashdump...");
	pragma::flush_loggers();

	std::string szResult;
	Locale::Load("prompts.txt");

	FileManager::CreateDirectory("crashdumps");

	// ask the user if they want to save a dump file
	auto saveDump = false;
	auto shouldShowMsBox = !engine->IsNonInteractiveMode();
#ifdef _WIN32
	shouldShowMsBox = (shouldShowMsBox && util::get_subsystem() == util::SubSystem::GUI);
#endif
	if(!shouldShowMsBox)
		saveDump = true;
	else {
		auto msg = Locale::GetText("prompt_crash");
		auto res = debug::show_message_prompt(msg, debug::MessageBoxButtons::YesNo, m_appName);
		saveDump = (res == debug::MessageBoxButton::Yes);
	}

#ifdef __linux__
	void *array[10];
	size_t size;
	char **symbols;
	char buffer[1024];
	buffer[0] = '\0';

	size = backtrace(array, 10);
	symbols = backtrace_symbols(array, size);

	std::optional<std::string> backtraceStr {};
	if (symbols != nullptr) {
		snprintf(buffer, sizeof(buffer), "Error: signal %d:\n", m_sig);

		for (size_t i = 0; i < size; i++) {
			strncat(buffer, symbols[i], sizeof(buffer) - strlen(buffer) - 1);
			strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
		}

		backtraceStr = buffer;
		free(symbols);
	}
#endif

	auto success = false;
	if(saveDump) {
		std::string err;
		std::string zipFileName;
		pragma::detail::close_logger();
		auto zipFile = Engine::GenerateEngineDump("crashdumps/crashdump", zipFileName, err);
		if(zipFile) {
#ifdef _WIN32
			std::string dumpErr;
			auto minidumpPath = GenerateMiniDump(dumpErr);
			if(minidumpPath) {
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
					f = FileManager::OpenSystemFile(minidumpPath->c_str(), "rb");
				}
				if(f != nullptr) {
					auto size = f->GetSize();
					std::vector<uint8_t> dumpData(size);
					f->Read(dumpData.data(), size);
					f = nullptr;
					zipFile->AddFile(ufile::get_file_from_filename(*minidumpPath), dumpData.data(), size);
					std::remove(minidumpPath->c_str());
				}
			}
			else
				zipFile->AddFile("minidump_generation_error.txt", dumpErr);
#else
			if (backtraceStr)
				zipFile->AddFile("backtrace.txt", *backtraceStr);
			else
				zipFile->AddFile("backtrace_generation_error.txt", "Failed to generate backtrace symbols");
#endif
			zipFile = nullptr;

			szResult = Locale::GetText("prompt_crash_dump_saved", std::vector<std::string> {zipFileName});
			auto absPath = util::Path::CreatePath(util::get_program_path()) + zipFileName;
			util::open_path_in_explorer(std::string {absPath.GetPath()}, std::string {absPath.GetFileName()});

			success = true;
		}
		else
			szResult = Locale::GetText("prompt_crash_dump_archive_failed", {err});
	}

	if(!szResult.empty() && shouldShowMsBox)
		debug::show_message_prompt(szResult, debug::MessageBoxButtons::Ok, m_appName);

	auto crashInProsperModule = false;
#ifdef _WIN32
	crashInProsperModule = pragma::debug::is_module_in_callstack(m_pExceptionInfo, "prosper");
#else
	crashInProsperModule = (backtraceStr && backtraceStr->find("libprosper") != std::string::npos);
#endif
	if(crashInProsperModule) {
		// Probably a rendering related crash.
		engine->HandleOpenGLFallback();
	}

	return success;
}

#ifdef _WIN32
LONG CrashHandler::TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	if(!g_crashHandler)
		return EXCEPTION_CONTINUE_SEARCH;
	g_crashHandler->m_pExceptionInfo = pExceptionInfo;
	g_crashHandler->GenerateCrashDump();
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif
