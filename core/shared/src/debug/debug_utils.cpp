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
#ifdef __linux__
#include "pragma/localization.h"
#endif

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
	if(!title)
		title = util::get_program_name();
#ifdef _WIN32
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

	auto wmsg = ustring::string_to_wstring(msg);
	auto wtitle = ustring::string_to_wstring(*title);
	auto res = ::MessageBoxW(NULL, wmsg.c_str(), wtitle.c_str(), winBt);
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
	std::vector<MessageBoxButton> buttons;
	buttons.reserve(3);
	switch(bts) {
	case MessageBoxButtons::Ok:
		buttons.push_back(MessageBoxButton::Ok);
		break;
	case MessageBoxButtons::OkCancel:
		buttons.push_back(MessageBoxButton::Ok);
		buttons.push_back(MessageBoxButton::Cancel);
		break;
	case MessageBoxButtons::AbortRetryIgnore:
		buttons.push_back(MessageBoxButton::Abort);
		buttons.push_back(MessageBoxButton::Retry);
		buttons.push_back(MessageBoxButton::Ignore);
		break;
	case MessageBoxButtons::YesNoCancel:
		buttons.push_back(MessageBoxButton::Yes);
		buttons.push_back(MessageBoxButton::No);
		buttons.push_back(MessageBoxButton::Cancel);
		break;
	case MessageBoxButtons::YesNo:
		buttons.push_back(MessageBoxButton::Yes);
		buttons.push_back(MessageBoxButton::No);
		break;
	case MessageBoxButtons::RetryCancel:
		buttons.push_back(MessageBoxButton::Retry);
		buttons.push_back(MessageBoxButton::Cancel);
		break;
	case MessageBoxButtons::CancelTryAgainContinue:
		buttons.push_back(MessageBoxButton::Cancel);
		buttons.push_back(MessageBoxButton::TryAgain);
		buttons.push_back(MessageBoxButton::Continue);
		break;
	}

	if(buttons.empty())
		return {};
	std::stringstream cmd;
	cmd<<"zenity ";
	if(buttons.size() == 1)
		cmd<<"--info ";
	else
		cmd<<"--question ";
	cmd<<"--title='" +*title +"' ";
	cmd<<"--text='" +msg +"' ";

	auto getButtonText = [](MessageBoxButton button) -> std::string {
		auto identifier = ustring::to_snake_case(std::string{magic_enum::enum_name(button)});
		auto text = Locale::GetText("prompt_button_" +identifier);
		return text;
	};
	cmd<<"--ok-label='"<<getButtonText(buttons[0])<<"' ";
	if(buttons.size() > 1) {
		cmd<<"--cancel-label='"<<getButtonText(buttons[1])<<"' ";
		if(buttons.size() > 2) {
			cmd<<"--extra-button='"<<getButtonText(buttons[2])<<"' ";
		}
	}

	int result = system(cmd.str().c_str());
	if(result < 0 || result >= buttons.size())
		return {};
	return buttons[result];
#endif
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
