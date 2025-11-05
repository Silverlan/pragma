// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.os;

export import pragma.lua;

export namespace Lua {
	namespace os {
		DLLNETWORK int64_t time_since_epoch(lua::State *l);
	};
};
