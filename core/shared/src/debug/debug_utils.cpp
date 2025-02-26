/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/debug/debug_utils.hpp"
#include <pragma/debug/intel_vtune.hpp>
#include <sharedutils/util.h>
#include <cstdio>
// #include <windows.h>
// #include <tlhelp32.h>
#ifdef __linux__
import pragma.locale;
#endif

void pragma::debug::start_profiling_task(const char *taskName)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask(taskName);
#endif
}
void pragma::debug::end_profiling_task()
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
}

void pragma::debug::open_file_in_zerobrane(const std::string &fileName, uint32_t lineIdx)
{
	std::string zeroBranePath = "C:/Program Files (x86)/ZeroBraneStudio/zbstudio.exe"; // TODO: Find program path from registry?
	util::start_process(zeroBranePath.c_str(), std::vector<std::string> {fileName + ':' + std::to_string(lineIdx)}, true);

	/*
	// Check if process is already running
	PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (Process32First(snapshot, &entry) == TRUE)
    {
        while (Process32Next(snapshot, &entry) == TRUE)
        {
            if (stricmp(entry.szExeFile, zeroBranePath.c_str()) == 0)
            {  
                HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);

				if(hProcess)
				{
					util::start_process(zeroBranePath.c_str(),std::vector<std::string>{
						fileName +':' +std::to_string(lineIdx)
					},true);
					// ...
					//GetExitCodeProcess(hProcess,exitCode);

				}

                CloseHandle(hProcess);
            }
        }
    }

    CloseHandle(snapshot);*/
}

#ifdef _WIN32
#include "debug/StackWalker/StackWalker.h"
class StackWalkerModuleFinder : public StackWalker {
  public:
	static bool find_module_in_callstack(PEXCEPTION_POINTERS exp, const std::string &moduleName)
	{
		StackWalkerModuleFinder finder {moduleName, exp};
		finder.ShowCallstack();
		return finder.m_found;
	}
  protected:
	StackWalkerModuleFinder(const std::string &moduleName, PEXCEPTION_POINTERS exp = NULL) : StackWalker {StackWalker::ExceptType::AfterCatch, OptionsAll, exp}, m_moduleName {moduleName} {}
	virtual void OnOutput(LPCSTR szText) {}
	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry) override
	{
		if(ustring::find(std::string {entry.moduleName}, m_moduleName, false))
			m_found = true;
	}
	std::string m_moduleName;
	bool m_found = false;
};
bool pragma::debug::is_module_in_callstack(struct _EXCEPTION_POINTERS *exp, const std::string &moduleName) { return StackWalkerModuleFinder ::find_module_in_callstack(exp, moduleName); }
#endif
