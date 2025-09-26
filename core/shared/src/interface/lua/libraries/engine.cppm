// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>

#undef LoadLibrary

export module pragma.client:scripting.lua.libraries.engine;

export namespace Lua {
	namespace engine {
		DLLNETWORK void PrecacheModel_sv(lua_State *l, const std::string &mdlName);
		DLLNETWORK void LoadSoundScripts(lua_State *l, const std::string &fileName, bool precache);
		DLLNETWORK void LoadSoundScripts(lua_State *l, const std::string &fileName);
		DLLNETWORK std::shared_ptr<Model> get_model(lua_State *l, const std::string &mdlName);
		DLLNETWORK var<bool, std::string> LoadLibrary(lua_State *l, const std::string &path);
		DLLNETWORK bool UnloadLibrary(lua_State *l, const std::string &path);
		DLLNETWORK bool IsLibraryLoaded(NetworkState &nw, const std::string &path);
		DLLNETWORK bool LibraryExists(lua_State *l, const std::string &library);
		DLLNETWORK uint64_t GetTickCount();
		DLLNETWORK tb<void> get_info(lua_State *l);
		DLLNETWORK opt<tb<void>> get_git_info(lua_State *l);
		DLLNETWORK void exit();
		DLLNETWORK std::string get_working_directory();

		DLLNETWORK void set_record_console_output(bool record);
		DLLNETWORK opt<mult<std::string, Con::MessageFlags, opt<Color>>> poll_console_output(lua_State *l);

		DLLNETWORK void register_shared_functions(lua_State *l, luabind::module_ &modEn);
	};
};
