// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:scripting.lua.libraries.animation;

export namespace Lua {
	namespace animation {
		DLLNETWORK void register_library(Lua::Interface &lua);
	};
};
