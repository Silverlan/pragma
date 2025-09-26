// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.client:scripting.lua.libraries.nav;

export namespace Lua {
	namespace nav {
		DLLNETWORK void register_library(Lua::Interface &lua);
	};
};
