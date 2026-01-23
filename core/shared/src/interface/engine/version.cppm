// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:engine.version;

export import pragma.util;

export namespace pragma {
	constexpr uint32_t ENGINE_VERSION_MAJOR = 1;
	constexpr uint32_t ENGINE_VERSION_MINOR = 3;
	constexpr uint32_t ENGINE_VERSION_REVISION = 0;

	DLLNETWORK util::Version &get_engine_version();
	DLLNETWORK std::string get_pretty_engine_version();
};
