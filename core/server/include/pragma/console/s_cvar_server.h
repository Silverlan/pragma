/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_CVAR_SERVER__
#define __S_CVAR_SERVER__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>

DLLSERVER void CMD_sv_send(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(sv_send,CMD_sv_send,ConVarFlags::None,"Sends a text message to all connected clients and displays it in the console. Usage: sv_send <message>");

DLLSERVER void CMD_sv_send_udp(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(sv_send_udp,CMD_sv_send_udp,ConVarFlags::None,"Sends a text message to all connected clients via UDP and displays it in the console. Usage: sv_send_udp <message>");

DLLSERVER void CMD_startserver(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(startserver,CMD_startserver,ConVarFlags::None,"Starts an internet server. Requires a running game.");

DLLSERVER void CMD_closeserver(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(closeserver,CMD_closeserver,ConVarFlags::None,"Closes the server (if active) and drops all connected clients.");

DLLSERVER void CMD_ent_input(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(ent_input,CMD_ent_input,ConVarFlags::None,"Triggers the given input on the specified entity. Usage: ent_input <entityName/entityClass> <input>");

DLLSERVER void CMD_ent_scale(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(ent_scale,CMD_ent_scale,ConVarFlags::None,"Changes the scale of the specified entity. Usage: ent_input <entityName/entityClass> <scale>");

DLLSERVER void CMD_ent_remove(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(ent_remove,CMD_ent_remove,ConVarFlags::None,"Removes the specified entity, or the entity the player is looking at if no argument is provided. Usage: ent_remove <entityName/className>");

DLLSERVER void CMD_ent_create(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(ent_create,CMD_ent_create,ConVarFlags::None,"Creates and spawns a new entity with the given class name at the position in the world the player is looking at. Usage: ent_create <className>");

DLLSERVER void CMD_nav_generate(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(nav_generate,CMD_nav_generate,ConVarFlags::None,"Generates a navigation mesh for the current map and saves it as a navigation file.");

DLLSERVER void CMD_nav_reload(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(nav_reload,CMD_nav_reload,ConVarFlags::None,"Reloads the navigation mesh for the current map.");

DLLSERVER void CMD_heartbeat(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(heartbeat,CMD_heartbeat,ConVarFlags::None,"Instantly sends a heartbeat to the master server.");

DLLSERVER void CMD_sv_debug_netmessages(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_SV(sv_debug_netmessages,CMD_sv_debug_netmessages,ConVarFlags::None,"Prints out debug information about recent net-messages.");

REGISTER_CONVAR_SV(sv_port_tcp,"29150",ConVarFlags::Archive,"TCP port which will be used when starting a server.");
REGISTER_CONVAR_SV(sv_port_udp,"29150",ConVarFlags::Archive,"UDP port which will be used when starting a server.");
REGISTER_CONVAR_SV(sv_use_p2p_if_available,"1",ConVarFlags::Archive,"Use a peer-to-peer connection if the selected networking layer supports it.");

#endif
