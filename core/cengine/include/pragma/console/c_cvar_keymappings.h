/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_CVAR_KEYMAPPINGS_H__
#define __C_CVAR_KEYMAPPINGS_H__
#include "pragma/c_enginedefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>

DLLCENGINE void CMD_bind_keys(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(bind_keys,CMD_bind_keys,ConVarFlags::None,"Prints a list of all bindable keys to the console.");

DLLCENGINE void CMD_bind(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(bind,CMD_bind,ConVarFlags::None,"Binds a key to a command string. Usage: bind <key> <command>");

DLLCENGINE void CMD_unbind(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(unbind,CMD_unbind,ConVarFlags::None,"Unbinds the given key.");

DLLCENGINE void CMD_unbindall(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(unbindall,CMD_unbindall,ConVarFlags::None,"Unbinds all keys.");

DLLCENGINE void CMD_keymappings(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(keymappings,CMD_keymappings,ConVarFlags::None,"Prints a list of all active key bindings to the console.");

#endif