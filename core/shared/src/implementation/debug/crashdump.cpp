// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#include <signal.h>
#else
#include <signal.h>
#include <execinfo.h>
#endif

#include "crashdump_helper.hpp"

module pragma.shared;

import :debug.crashdump;
import :locale;
import util_zip;

#undef CreateDirectory

using namespace pragma::debug;

static spdlog::logger &LOGGER = pragma::register_logger("crash_handler");

static std::string g_crashExceptionMessage = {};
static CrashHandler g_crashHandler {};

const std::string &pragma::debug::get_exception_message() { return g_crashExceptionMessage; }

bool pragma::debug::generate_crash_dump() { return g_crashHandler.GenerateCrashDump(); }

CrashHandler &CrashHandler::Get() { return g_crashHandler; }

void CrashHandler::Initialize() { Get().InitializeHandlers(); }

CrashHandler::CrashHandler() { InitializeHandlers(); }
void CrashHandler::InitializeHandlers()
{
#ifdef _WIN32
	::SetUnhandledExceptionFilter(TopLevelFilter);
	_set_invalid_parameter_handler([](const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int line, uintptr_t pReserved) {
		__try {
			// Trigger an exception
			int *p = nullptr;
			*p = 42;
		}
		__except(CrashHandler::GenerateCrashDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER) {
		}
		exit(1);
	});
	_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
	_set_purecall_handler([]() {
		__try {
			// Trigger an exception
			int *p = nullptr;
			*p = 42;
		}
		__except(CrashHandler::GenerateCrashDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER) {
		}
		exit(1);
	});
	signal(SIGABRT, [](int signal) {
		__try {
			// Trigger an exception
			int *p = nullptr;
			*p = 42;
		}
		__except(CrashHandler::GenerateCrashDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER) {
		}
		exit(signal);
	});
#else
	signal(
	  SIGSEGV, +[](int sig) {
		  LOGGER.critical("SIGSEV {}", sig);
		  g_crashHandler.m_sig = sig;
		  g_crashHandler.GenerateCrashDump();
		  exit(1);
	  });
#endif
	// Note: set_terminate handler is called before SetUnhandledExceptionFilter.
	// set_terminate allows us to retrieve the underlying message from the exception (if there was one)
	std::set_terminate([]() {
		LOGGER.critical("terminate handler invoked...");
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
#ifdef __linux__
		g_crashHandler.GenerateCrashDump();
#else
		// Relay exception to SetUnhandledExceptionFilter
		std::rethrow_exception(eptr);
#endif
	});
}

void CrashHandler::SetAppName(const std::string &appName) { m_appName = appName; }

CrashHandler::~CrashHandler() {}

#ifdef _WIN32
BOOL CALLBACK MyMiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput);

std::optional<std::string> CrashHandler::GenerateMiniDump(std::string &outErr) const
{
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

	auto programPath = fs::get_program_write_path();
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

	MINIDUMP_CALLBACK_INFORMATION mci;

	mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback;
	mci.CallbackParam = 0;

	// write the dump
	// Add MiniDumpWithDataSegs if global variables are needed
	MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);
	BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, &ExInfo, NULL, &mci);
	::CloseHandle(hFile);
	if(bOK)
		return szDumpPath;
	outErr = "Failed to write minidump!";
	return {};
}

///////////////////////////////////////////////////////////////////////////////
// Custom minidump callback
//

BOOL CALLBACK MyMiniDumpCallback(PVOID pParam, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput)
{
	BOOL bRet = FALSE;

	// Check parameters

	if(pInput == 0)
		return FALSE;

	if(pOutput == 0)
		return FALSE;

	// Process the callbacks

	switch(pInput->CallbackType) {
	case IncludeModuleCallback:
		{
			// Include the module into the dump
			bRet = TRUE;
		}
		break;

	case IncludeThreadCallback:
		{
			// Include the thread into the dump
			bRet = TRUE;
		}
		break;

	case ModuleCallback:
		{
			// Does the module have ModuleReferencedByMemory flag set ?

			if(!(pOutput->ModuleWriteFlags & ModuleReferencedByMemory)) {
				// No, it does not - exclude it

				pOutput->ModuleWriteFlags &= (~ModuleWriteModule);
			}

			bRet = TRUE;
		}
		break;

	case ThreadCallback:
		{
			// Include all thread information into the minidump
			bRet = TRUE;
		}
		break;

	case ThreadExCallback:
		{
			// Include this information
			bRet = TRUE;
		}
		break;

	case MemoryCallback:
		{
			// We do not include any information here -> return FALSE
			bRet = FALSE;
		}
		break;

	case CancelCallback:
		break;
	}

	return bRet;
}

bool CrashHandler::GenerateCrashDump(EXCEPTION_POINTERS *pExceptionPointers)
{
	g_crashHandler.m_pExceptionInfo = pExceptionPointers;
	return g_crashHandler.GenerateCrashDump();
}
#endif

bool CrashHandler::GenerateCrashDump() const
{
	std::cout << "Generating crashdump..." << std::endl;
	LOGGER.info("Generating crashdump...");
	flush_loggers();

	LOGGER.debug("Loading localization files...");
	std::string szResult;
	locale::load("prompts.txt");

	LOGGER.debug("Creating 'crashdumps' directory...");
	fs::create_directory("crashdumps");

	// ask the user if they want to save a dump file
	auto saveDump = false;
	auto *engine = get_engine();
	auto shouldShowMsBox = !Engine::Get()->IsNonInteractiveMode();
#ifdef _WIN32
	shouldShowMsBox = (shouldShowMsBox && pragma::util::get_subsystem() == pragma::util::SubSystem::GUI);
#endif
	if(!shouldShowMsBox)
		saveDump = true;
	else {
		LOGGER.debug("Displaying prompt message...");
		auto msg = locale::get_text("prompt_crash");
		auto res = show_message_prompt(msg, MessageBoxButtons::YesNo, m_appName);
		// If res is nullopt, the message prompt most likely failed to show. In this case we'll assume yes as
		// default answer.
		saveDump = (res == std::nullopt || *res == MessageBoxButton::Yes);
	}

#ifdef __linux__
	LOGGER.debug("Generating stack backtrace...");
	void *array[10];
	size_t size;
	char **symbols;
	char buffer[1024];
	buffer[0] = '\0';

	size = backtrace(array, 10);
	symbols = backtrace_symbols(array, size);

	std::optional<std::string> backtraceStr {};
	if(symbols != nullptr) {
		snprintf(buffer, sizeof(buffer), "Error: signal %d:\n", m_sig);

		for(size_t i = 0; i < size; i++) {
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
		auto zipFile = Engine::GenerateEngineDump("crashdumps/crashdump", zipFileName, err);
		// Logger should already be closed at this point, but to make sure...
		detail::close_logger();
		if(zipFile) {
#ifdef _WIN32
			std::string dumpErr;
			auto minidumpPath = GenerateMiniDump(dumpErr);
			if(minidumpPath) {
				// Write Minidump
				fs::VFilePtrReal f = nullptr;
				auto t = pragma::util::Clock::now();
				while(f == nullptr) // Wait until dump has been written
				{
					auto tNow = pragma::util::Clock::now();
					auto tDelta = std::chrono::duration_cast<std::chrono::seconds>(tNow - t).count();
					if(tDelta >= 4) // Don't wait more than 4 seconds
						break;
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
					f = fs::open_system_file(*minidumpPath, fs::FileMode::Read | fs::FileMode::Binary);
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
			if(backtraceStr)
				zipFile->AddFile("backtrace.txt", *backtraceStr);
			else
				zipFile->AddFile("backtrace_generation_error.txt", "Failed to generate backtrace symbols");
#endif
			zipFile = nullptr;

			szResult = locale::get_text("prompt_crash_dump_saved", std::vector<std::string> {zipFileName, "crashdumps@pragma-engine.com"});
			std::string absPath;
			if(fs::find_absolute_path(zipFileName, absPath)) {
				auto path = util::FilePath(absPath);
				util::open_path_in_explorer(std::string {path.GetPath()}, std::string {path.GetFileName()});
			}

			success = true;
		}
		else
			szResult = locale::get_text("prompt_crash_dump_archive_failed", {err});
	}

	if(!szResult.empty() && shouldShowMsBox)
		show_message_prompt(szResult, MessageBoxButtons::Ok, m_appName);

	auto crashInProsperModule = false;
#ifdef _WIN32
	crashInProsperModule = pragma::debug::is_module_in_callstack(m_pExceptionInfo, "prosper");
#else
	crashInProsperModule = (backtraceStr && backtraceStr->find("libprosper") != std::string::npos);
#endif
	if(crashInProsperModule) {
		// Probably a rendering related crash.
		Engine::Get()->HandleOpenGLFallback();
	}

	// We've done all we can, just force quit at this point
	exit(1);

	// Unreachable
	return success;
}

#ifdef _WIN32
static auto g_crashDumpGenerated = false;
static std::mutex g_crashMutex;
LONG CrashHandler::TopLevelFilter(_EXCEPTION_POINTERS *pExceptionInfo)
{
	// When a crash occurs, there's a good chance that other threads will also crash.
	// This can cause the entire application to die before the crash dump has been fully generated.
	// Since we probably only care about the first crash anyway, we'll simply make all other threads wait
	// until the crash dump has been generated.
	std::unique_lock<std::mutex> lock {g_crashMutex};
	if(g_crashDumpGenerated)
		return EXCEPTION_CONTINUE_SEARCH;

	g_crashDumpGenerated = true;
	g_crashHandler.m_pExceptionInfo = pExceptionInfo;
	g_crashHandler.GenerateCrashDump();
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif
