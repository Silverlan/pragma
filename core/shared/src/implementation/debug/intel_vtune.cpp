// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
#include <ittnotify.h>
#endif

module pragma.shared;

import :debug.intel_vtune;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING

debug::VTuneDomain::VTuneDomain()
{
	m_domain = __itt_domain_create("Pragma");
	__itt_thread_set_name("Pragma - Main");
}

debug::VTuneDomain::~VTuneDomain() {}

debug::VTuneTask debug::VTuneDomain::BeginTask(const std::string &name)
{
	auto *handle = __itt_string_handle_create(name.c_str());
	__itt_task_begin(m_domain, __itt_null, __itt_null, handle);
	return debug::VTuneTask {m_domain, nullptr};
}

void debug::VTuneDomain::EndTask() { __itt_task_end(m_domain); }

void debug::VTuneTask::Begin() { __itt_task_begin(domain, __itt_null, __itt_null, handle); }

void debug::VTuneTask::End() { __itt_task_end(domain); }

static std::unique_ptr<debug::VTuneDomain> g_domain = nullptr;
debug::VTuneDomain &debug::open_domain()
{
	if(!g_domain)
		g_domain = std::make_unique<debug::VTuneDomain>();
	return *g_domain;
}
debug::VTuneDomain &debug::get_domain() { return *g_domain; }
void debug::close_domain() { g_domain = nullptr; }
#endif
