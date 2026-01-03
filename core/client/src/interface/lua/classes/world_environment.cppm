// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.world_environment;

export import :rendering.world_environment;

export namespace Lua {
	namespace WorldEnvironment {
		DLLCLIENT void register_class(luabind::class_<pragma::rendering::WorldEnvironment> &classDef);
	};
};
