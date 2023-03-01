/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/debug/debug_lua_zerobrane.hpp"
#include <sharedutils/util.h>
#include <cstdio>
// #include <windows.h>
// #include <tlhelp32.h>

void debug::open_file_in_zerobrane(const std::string &fileName, uint32_t lineIdx)
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
