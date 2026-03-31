// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
#include <ittnotify.h>
#endif

module pragma.shared;

import :debug.intel_vtune;

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING

pragma::debug::VTuneDomain::VTuneDomain()
{
	m_domain = __itt_domain_create("Pragma");
	__itt_thread_set_name("Pragma - Main");
}

pragma::debug::VTuneDomain::~VTuneDomain() {}

pragma::debug::VTuneTask pragma::debug::VTuneDomain::BeginTask(const std::string &name)
{
	auto *handle = __itt_string_handle_create(name.c_str());
	__itt_task_begin(m_domain, __itt_null, __itt_null, handle);
	return debug::VTuneTask {m_domain, nullptr};
}

void pragma::debug::VTuneDomain::EndTask() { __itt_task_end(m_domain); }

void pragma::debug::VTuneTask::Begin() { __itt_task_begin(domain, __itt_null, __itt_null, handle); }

void pragma::debug::VTuneTask::End() { __itt_task_end(domain); }

static std::unique_ptr<pragma::debug::VTuneDomain> g_domain = nullptr;
pragma::debug::VTuneDomain &pragma::debug::open_domain()
{
	if(!g_domain)
		g_domain = std::make_unique<debug::VTuneDomain>();
	return *g_domain;
}
pragma::debug::VTuneDomain &pragma::debug::get_domain() { return *g_domain; }
void pragma::debug::close_domain() { g_domain = nullptr; }
#endif
