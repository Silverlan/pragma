// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.net;

export import :entities.enums;
export import pragma.lua;

export namespace Lua {
	namespace net {
		DLLNETWORK void RegisterLibraryEnums(lua::State *l);
		DLLNETWORK pragma::NetEventId register_event(lua::State *l, const std::string &name);
	};
};
