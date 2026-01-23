// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.convar;

export import :console.convar;
export import :entities.components.base_player;
export import :scripting.lua.api;
export import :scripting.lua.types;

export {
	namespace Lua::console {
		pragma::console::ConVar *CreateConVar(lua::State *l, const std::string &cmd, udm::Type type, udm_type def, pragma::console::ConVarFlags flags = pragma::console::ConVarFlags::None, const std::string &help = "");
		void CreateConCommand(lua::State *l, const std::string &name, const func<void, pragma::BasePlayerComponent, float, variadic<std::string>> &function, pragma::console::ConVarFlags flags, const std::string &help);
		void CreateConCommand(lua::State *l, const std::string &name, const func<void, pragma::BasePlayerComponent, float, variadic<std::string>> &function, pragma::console::ConVarFlags flags);
		void CreateConCommand(lua::State *l, const std::string &name, const func<void, pragma::BasePlayerComponent, float, variadic<std::string>> &function, const std::string &help);
		pragma::console::ConVar *GetConVar(lua::State *l, const std::string &name);
		int32_t GetConVarInt(lua::State *l, const std::string &conVar);
		float GetConVarFloat(lua::State *l, const std::string &conVar);
		std::string GetConVarString(lua::State *l, const std::string &conVar);
		bool GetConVarBool(lua::State *l, const std::string &conVar);
		pragma::console::ConVarFlags GetConVarFlags(lua::State *l, const std::string &conVar);
		DLLNETWORK int Run(lua::State *l);
		DLLNETWORK int AddChangeCallback(lua::State *l);
	};

	////////////////////////////////////

	namespace Lua {
		namespace console {
			DLLNETWORK void register_override(lua::State *l, const std::string &src, const std::string &dst);
			DLLNETWORK void clear_override(lua::State *l, const std::string &src);
			DLLNETWORK int parse_command_arguments(lua::State *l);
		};
	};
};
