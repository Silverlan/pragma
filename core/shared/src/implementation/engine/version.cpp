// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_shared.h"
#include "pragma/engine_version.h"
#include <sharedutils/util_version.h>

module pragma.shared;

import :version;

static util::Version engine_version(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_REVISION);
util::Version &get_engine_version() { return engine_version; }

std::string get_pretty_engine_version()
{
	auto &version = get_engine_version();
	std::stringstream ssVersion;
	ssVersion << "v" << version.ToString() << " "
#ifdef _WIN32
#ifdef _M_X64
	          << "Win64"
#else
	          << "Win32"
#endif
#else
#if INTPTR_MAX == INT64_MAX
	          << "Lin64"
#else
	          << "Lin32"
#endif
#endif
#ifdef _DEBUG
	          << " (Debug)"
#endif
	  ;
	return ssVersion.str();
}
