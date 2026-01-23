// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.ai;

export import pragma.lua;

export namespace Lua {
	namespace ai {
		namespace client {
			DLLCLIENT void register_library(Interface &lua);
		};
	};
};
