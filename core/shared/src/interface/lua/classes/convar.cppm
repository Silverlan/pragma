// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

export module pragma.shared:scripting.lua.classes.convar;

export {
	namespace Lua::console {
		ConVar *CreateConVar(lua_State *l, const std::string &cmd, ::udm::Type type, Lua::udm_type def, ConVarFlags flags = ConVarFlags::None, const std::string &help = "");
		void CreateConCommand(lua_State *l, const std::string &name, const Lua::func<void, pragma::BasePlayerComponent, float, Lua::variadic<std::string>> &function, ConVarFlags flags, const std::string &help);
		void CreateConCommand(lua_State *l, const std::string &name, const Lua::func<void, pragma::BasePlayerComponent, float, Lua::variadic<std::string>> &function, ConVarFlags flags);
		void CreateConCommand(lua_State *l, const std::string &name, const Lua::func<void, pragma::BasePlayerComponent, float, Lua::variadic<std::string>> &function, const std::string &help);
		ConVar *GetConVar(lua_State *l, const std::string &name);
		int32_t GetConVarInt(lua_State *l, const std::string &conVar);
		float GetConVarFloat(lua_State *l, const std::string &conVar);
		std::string GetConVarString(lua_State *l, const std::string &conVar);
		bool GetConVarBool(lua_State *l, const std::string &conVar);
		ConVarFlags GetConVarFlags(lua_State *l, const std::string &conVar);
		DLLNETWORK int Run(lua_State *l);
		DLLNETWORK int AddChangeCallback(lua_State *l);
	};

	////////////////////////////////////

	namespace Lua {
		namespace console {
			DLLNETWORK void register_override(lua_State *l, const std::string &src, const std::string &dst);
			DLLNETWORK void clear_override(lua_State *l, const std::string &src);
			DLLNETWORK int parse_command_arguments(lua_State *l);
		};
	};
};
