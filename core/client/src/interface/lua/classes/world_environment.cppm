// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:scripting.lua.classes.world_environment;

import :rendering.world_environment;

export namespace Lua {
	namespace WorldEnvironment {
		DLLCLIENT void register_class(luabind::class_<::WorldEnvironment> &classDef);
	};
};
