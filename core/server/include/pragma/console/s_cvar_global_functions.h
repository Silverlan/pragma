/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_CVAR_GLOBAL_FUNCTIONS_H__
#define __S_CVAR_GLOBAL_FUNCTIONS_H__

#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <vector>
#include <string>

class NetworkState;
DLLSERVER void CMD_lua_run(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&);
DLLSERVER void CMD_lua_exec(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&);
DLLSERVER void CMD_entities_sv(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&);
DLLSERVER void CMD_map(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&);
DLLSERVER void CMD_list_maps(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&);
DLLSERVER void CMD_status_sv(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string>&);
DLLSERVER void CMD_drop(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
DLLSERVER void CMD_kick(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
#ifdef _DEBUG
DLLSERVER void CMD_sv_dump_netmessages(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
#endif

#endif
