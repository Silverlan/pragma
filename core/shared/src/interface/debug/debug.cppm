// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#ifdef _WIN32
#include <Windows.h>
#endif

export module pragma.shared:debug;

export import :debug.behavior_tree_node;
export import :debug.crashdump;
export import :debug.intel_vtune;
export import :debug.performance_profiler;
export import :debug.render_info;

export namespace pragma::debug {
	DLLNETWORK void open_file_in_zerobrane(const std::string &fileName, uint32_t lineIdx);
#ifdef _WIN32
	DLLNETWORK bool is_module_in_callstack(_EXCEPTION_POINTERS *exp, const std::string &moduleName);
#endif
	DLLNETWORK void start_profiling_task(const char *taskName);
	DLLNETWORK void end_profiling_task();
};
