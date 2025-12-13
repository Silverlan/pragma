// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.nav;

export import :scripting.lua.api;

export namespace Lua {
	namespace nav {
		DLLNETWORK void register_library(Interface &lua);
	};
};
