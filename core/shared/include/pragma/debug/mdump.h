/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __MDUMP_H__
#define __MDUMP_H__

#include "pragma/definitions.h"

#ifdef _WIN32
#if _MSC_VER < 1300
#define DECLSPEC_DEPRECATED
// VC6: change this path to your Platform SDK headers
#include "M:\\dev7\\vs\\devtools\\common\\win32sdk\\include\\dbghelp.h" // must be XP version of file
#else
// VC7: ships with updated headers
#include "dbghelp.h"
#endif

// based on dbghelp.h
typedef BOOL(WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);
#endif

namespace pragma::debug {
	class DLLNETWORK CrashHandler {
	  public:
		CrashHandler(const std::string &appName);
		~CrashHandler();
	  private:
		bool GenerateCrashDump() const;
		std::string m_appName;
#ifdef _WIN32
		struct _EXCEPTION_POINTERS *m_pExceptionInfo = nullptr;
		std::optional<std::string> GenerateMiniDump(std::string &outErr) const;
		static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);
#else
		int m_sig = -1;
#endif
	};
};
#endif
