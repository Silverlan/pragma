#ifndef __S_NET_GLOBAL_H__
#define __S_NET_GLOBAL_H__
#include "pragma/networking/s_net_definitions.h"
DECLARE_NETMESSAGE_SV(disconnect);
DECLARE_NETMESSAGE_SV(userinput);
DECLARE_NETMESSAGE_SV(clientinfo);
DECLARE_NETMESSAGE_SV(game_ready);
DECLARE_NETMESSAGE_SV(cmd_call);
DECLARE_NETMESSAGE_SV(rcon);
DECLARE_NETMESSAGE_SV(serverinfo_request);
DECLARE_NETMESSAGE_SV(authenticate);
DECLARE_NETMESSAGE_SV(luanet);
DECLARE_NETMESSAGE_SV(cmd_setpos);
DECLARE_NETMESSAGE_SV(cvar_set);
DECLARE_NETMESSAGE_SV(noclip);
DECLARE_NETMESSAGE_SV(notarget);
DECLARE_NETMESSAGE_SV(godmode);
DECLARE_NETMESSAGE_SV(suicide);
DECLARE_NETMESSAGE_SV(hurtme);
DECLARE_NETMESSAGE_SV(weapon_next);
DECLARE_NETMESSAGE_SV(weapon_previous);
DECLARE_NETMESSAGE_SV(ent_event);
DECLARE_NETMESSAGE_SV(give_weapon);
DECLARE_NETMESSAGE_SV(give_ammo);
DECLARE_NETMESSAGE_SV(debug_ai_navigation);
DECLARE_NETMESSAGE_SV(debug_ai_schedule_print);
DECLARE_NETMESSAGE_SV(debug_ai_schedule_tree);
#endif
