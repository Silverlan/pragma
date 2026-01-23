// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.libraries.import_lib;

export import pragma.lua;

export namespace Lua {
	namespace lib_export {
		DLLCLIENT int export_scene(lua::State *l);
	};
};
