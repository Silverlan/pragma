#ifndef __C_CVAR_SERVER__
#define __C_CVAR_SERVER__
#include "pragma/clientdefinitions.h"
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>
#include "pragma/networking/portinfo.h"

REGISTER_CONVAR_CL(playername,"player",ConVarFlags::Archive | ConVarFlags::Userinfo,"Local player name.");
REGISTER_CONVAR_CL(password,"",ConVarFlags::None,"Password which will be used for the next connection attempt.");
REGISTER_CONVAR_CL(cl_updaterate,"20",ConVarFlags::Archive,"The amount of times per second user input is being transmitted to the server.");
REGISTER_CONVAR_CL(net_graph,"0",ConVarFlags::None,"Displays a graph about current network transmissions.");

DLLCLIENT void CMD_cl_send(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(cl_send,CMD_cl_send,ConVarFlags::None,"Sends a text message to the server and displays it in the console. Usage: cl_send <message>");

DLLCLIENT void CMD_cl_send_udp(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(cl_send_udp,CMD_cl_send_udp,ConVarFlags::None,"Sends a text message to the server via UDP and displays it in the console. Usage: cl_send_udp <message>");

DLLCLIENT void CMD_cl_rcon(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(rcon,CMD_cl_rcon,ConVarFlags::None,"Sends the given command to the server and runs it serverside. Usage: rcon <command>");

DLLCLIENT void CMD_connect(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(connect,CMD_connect,ConVarFlags::None,"Attempts to connect to the specified server. Usage: connect <ip>");

DLLCLIENT void CMD_disconnect(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(disconnect,CMD_disconnect,ConVarFlags::None,"Disconnects from the server (if a connection is active), or closes the game if in single player mode.");

DLLCLIENT void CMD_cl_debug_netmessages(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv);
REGISTER_CONCOMMAND_CL(cl_debug_netmessages,CMD_cl_debug_netmessages,ConVarFlags::None,"Prints out debug information about recent net-messages.");

REGISTER_CONVAR_CL(cl_port_tcp,sci::DEFAULT_PORT_TCP,ConVarFlags::Archive | ConVarFlags::Userinfo,"Port used for TCP transmissions.");
REGISTER_CONVAR_CL(cl_port_udp,sci::DEFAULT_PORT_UDP,ConVarFlags::Archive | ConVarFlags::Userinfo,"Port used for UDP transmissions.");

REGISTER_CONVAR_CL(cl_max_fps,"-1",ConVarFlags::Archive,"FPS will be clamped at this value. A value of < 0 deactivates the limit.");
#endif
