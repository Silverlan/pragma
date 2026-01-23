// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :engine.version;

static pragma::util::Version engine_version(pragma::ENGINE_VERSION_MAJOR, pragma::ENGINE_VERSION_MINOR, pragma::ENGINE_VERSION_REVISION);
pragma::util::Version &pragma::get_engine_version() { return engine_version; }

std::string pragma::get_pretty_engine_version()
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
