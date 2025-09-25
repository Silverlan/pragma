// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.shared:debug;
export import pragma.shared:debug.behavior_tree_node;
export import pragma.shared:debug.crashdump;
export import pragma.shared:debug.intel_vtune;
export import pragma.shared:debug.performance_profiler;
export import pragma.shared:debug.render_info;

export namespace pragma::debug {
	DLLNETWORK void open_file_in_zerobrane(const std::string &fileName, uint32_t lineIdx);
#ifdef _WIN32
	DLLNETWORK bool is_module_in_callstack(struct _EXCEPTION_POINTERS *exp, const std::string &moduleName);
#endif
	DLLNETWORK void start_profiling_task(const char *taskName);
	DLLNETWORK void end_profiling_task();
};
