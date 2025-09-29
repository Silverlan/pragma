// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"


export module pragma.client:scripting.lua.libraries.ai;
export namespace Lua {
	namespace ai {
		namespace client {
			DLLCLIENT void register_library(Lua::Interface &lua);
		};
	};
};
