/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/debug/debug_utils.hpp"
#include <sharedutils/util.h>
#include <cstdio>
// #include <windows.h>
// #include <tlhelp32.h>

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

std::optional<pragma::debug::MessageBoxButton> pragma::debug::show_message_prompt(const std::string &msg, MessageBoxButtons bts, std::optional<std::string> title)
{
#ifdef _WIN32
	if(!title)
		title = util::get_program_name();
	uint32_t winBt = 0;
	switch(bts) {
	case MessageBoxButtons::Ok:
		winBt = MB_OK;
		break;
	case MessageBoxButtons::OkCancel:
		winBt = MB_OKCANCEL;
		break;
	case MessageBoxButtons::AbortRetryIgnore:
		winBt = MB_ABORTRETRYIGNORE;
		break;
	case MessageBoxButtons::YesNoCancel:
		winBt = MB_YESNOCANCEL;
		break;
	case MessageBoxButtons::YesNo:
		winBt = MB_YESNO;
		break;
	case MessageBoxButtons::RetryCancel:
		winBt = MB_RETRYCANCEL;
		break;
	case MessageBoxButtons::CancelTryAgainContinue:
		winBt = MB_CANCELTRYCONTINUE;
		break;
	}

	auto res = ::MessageBox(NULL, msg.c_str(), title->c_str(), winBt);
	switch(res) {
	case IDOK:
		return MessageBoxButton::Ok;
	case IDCANCEL:
		return MessageBoxButton::Cancel;
	case IDABORT:
		return MessageBoxButton::Abort;
	case IDRETRY:
		return MessageBoxButton::Retry;
	case IDIGNORE:
		return MessageBoxButton::Ignore;
	case IDYES:
		return MessageBoxButton::Yes;
	case IDNO:
		return MessageBoxButton::No;
	case IDTRYAGAIN:
		return MessageBoxButton::TryAgain;
	case IDCONTINUE:
		return MessageBoxButton::Continue;
	default:
		return {};
	}
	return {};
#else
	return {};
#endif
}
