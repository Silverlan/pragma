// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "debug/crashdump_helper.hpp"
#include <atomic>

export module pragma.debug.crashdump;

export namespace pragma::debug {
	DLLNETWORK const std::string &get_exception_message();
	DLLNETWORK bool generate_crash_dump();
	class DLLNETWORK CrashHandler {
	  public:
		static CrashHandler &Get();
		static void Initialize();
		CrashHandler();
		~CrashHandler();
		void SetAppName(const std::string &appName);
	  private:
		friend bool generate_crash_dump();
		// May be called multiple times if re-initialization is needed
		void InitializeHandlers();
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
