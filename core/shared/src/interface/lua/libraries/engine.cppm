// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.libraries.engine;

import :console.enums;
export import :scripting.lua.api;
export import :types;

#undef LoadLibrary

export {
	namespace Lua {
		namespace engine {
			DLLNETWORK void PrecacheModel_sv(lua::State *l, const std::string &mdlName);
			DLLNETWORK void LoadSoundScripts(lua::State *l, const std::string &fileName, bool precache);
			DLLNETWORK void LoadSoundScripts(lua::State *l, const std::string &fileName);
			DLLNETWORK std::shared_ptr<pragma::asset::Model> get_model(lua::State *l, const std::string &mdlName);
			DLLNETWORK var<bool, std::string> LoadLibrary(lua::State *l, const std::string &path);
			DLLNETWORK bool UnloadLibrary(lua::State *l, const std::string &path);
			DLLNETWORK bool IsLibraryLoaded(pragma::NetworkState &nw, const std::string &path);
			DLLNETWORK bool LibraryExists(lua::State *l, const std::string &library);
			DLLNETWORK uint64_t GetTickCount();
			DLLNETWORK tb<void> get_info(lua::State *l);
			DLLNETWORK opt<tb<void>> get_git_info(lua::State *l);
			DLLNETWORK void exit();
			DLLNETWORK std::string get_working_directory();

			DLLNETWORK void set_record_console_output(bool record);
			DLLNETWORK opt<mult<std::string, pragma::console::MessageFlags, opt<Color>>> poll_console_output(lua::State *l);

			DLLNETWORK void register_shared_functions(lua::State *l, luabind::module_ &modEn);
		};
	};
};
