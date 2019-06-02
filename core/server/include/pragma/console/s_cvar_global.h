#ifndef __S_CVAR_GLOBAL_H__
#define __S_CVAR_GLOBAL_H__

#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>
#include "pragma/console/s_cvar_global_functions.h"

REGISTER_CONCOMMAND_SV(lua_run,CMD_lua_run,ConVarFlags::None,"Runs a lua command on the server lua state.");
REGISTER_CONCOMMAND_SV(entities,CMD_entities_sv,ConVarFlags::None,"Prints a list of all current serverside entities in the world.");
REGISTER_CONCOMMAND_SV(list_maps,CMD_list_maps,ConVarFlags::None,"Prints a list of all available list to the console.");
REGISTER_CONCOMMAND_SV(status,CMD_status_sv,ConVarFlags::None,"Prints information about the server to the console.");
REGISTER_CONCOMMAND_SV(drop,CMD_drop,ConVarFlags::None,"Drops the player's active weapon.");
REGISTER_CONCOMMAND_SV(kick,CMD_kick,ConVarFlags::None,"Kicks the specified player for the given reason. Usage: kick <playerId/playerName> <reason>");
#ifdef _DEBUG
REGISTER_CONCOMMAND_SV(sv_dump_netmessages,CMD_sv_dump_netmessages,ConVarFlags::None,"Prints all registered netmessages to the console.");
#endif

#endif