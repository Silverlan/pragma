// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.ai;

export import pragma.lua;

export namespace Lua {
	namespace ai {
		DLLNETWORK void register_library(Interface &lua);
	};
};
