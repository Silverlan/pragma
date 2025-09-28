// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_EN_CVAR_GLOBAL_H__
#define __C_EN_CVAR_GLOBAL_H__

DLLCLIENT void CMD_lua_reload_entity(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(lua_reload_entity, CMD_lua_reload_entity, ConVarFlags::None, "Reloads the scripts for the given entity class. Usage: lua_reload_entity <className>");

DLLCLIENT void CMD_lua_reload_weapon(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(lua_reload_weapon, CMD_lua_reload_weapon, ConVarFlags::None, "Reloads the scripts for the given weapon class. Usage: lua_reload_weapon <className>");

DLLCLIENT void CMD_lua_reload_entities(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(lua_reload_entities, CMD_lua_reload_entities, ConVarFlags::None, "Reloads the scripts for all registered lua entity classes.");

DLLCLIENT void CMD_lua_reload_weapons(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(lua_reload_weapons, CMD_lua_reload_weapons, ConVarFlags::None, "Reloads the scripts for all registered lua weapon classes.");
#endif
