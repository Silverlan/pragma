/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

module;

#include "debug/crashdump_helper.hpp"

export module pragma.debug.crashdump;

export namespace pragma::debug {
	DLLNETWORK const std::string &get_exception_message();
	class DLLNETWORK CrashHandler {
	  public:
		static CrashHandler &Get();
		CrashHandler();
		~CrashHandler();
		void SetAppName(const std::string &appName);
	  private:
		bool GenerateCrashDump() const;
		std::string m_appName;
#ifdef _WIN32
		static bool GenerateCrashDump(EXCEPTION_POINTERS *pExceptionPointers);
		struct _EXCEPTION_POINTERS *m_pExceptionInfo = nullptr;
		std::optional<std::string> GenerateMiniDump(std::string &outErr) const;
		static LONG WINAPI TopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);
#else
		int m_sig = -1;
#endif
	};
};
