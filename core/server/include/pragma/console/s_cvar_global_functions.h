// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_CVAR_GLOBAL_FUNCTIONS_H__
#define __S_CVAR_GLOBAL_FUNCTIONS_H__

#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <vector>
#include <string>

class NetworkState;
DLLSERVER void CMD_entities_sv(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
DLLSERVER void CMD_map(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
DLLSERVER void CMD_list_maps(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
DLLSERVER void CMD_status_sv(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
DLLSERVER void CMD_drop(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLSERVER void CMD_kick(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#ifdef _DEBUG
DLLSERVER void CMD_sv_dump_netmessages(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#endif

#endif
