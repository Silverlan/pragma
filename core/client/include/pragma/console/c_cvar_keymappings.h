// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_CVAR_KEYMAPPINGS_H__
#define __C_CVAR_KEYMAPPINGS_H__
#include "pragma/c_enginedefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>

DLLCLIENT void CMD_bind_keys(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(bind_keys, CMD_bind_keys, ConVarFlags::None, "Prints a list of all bindable keys to the console.");

DLLCLIENT void CMD_bind(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(bind, CMD_bind, ConVarFlags::None, "Binds a key to a command string. Usage: bind <key> <command>");

DLLCLIENT void CMD_unbind(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(unbind, CMD_unbind, ConVarFlags::None, "Unbinds the given key.");

DLLCLIENT void CMD_unbindall(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(unbindall, CMD_unbindall, ConVarFlags::None, "Unbinds all keys.");

DLLCLIENT void CMD_keymappings(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(keymappings, CMD_keymappings, ConVarFlags::None, "Prints a list of all active key bindings to the console.");

#endif
