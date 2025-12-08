// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :engine.version;
import :networking.net_message_map;
import :networking.net_messages;

static void register_server_net_messages(pragma::networking::IBaseNetMessageMap &netMessageMap)
{
	using namespace pragma::networking::net_messages;
	netMessageMap.RegisterNetMessage(server::RESOURCEINFO_RESPONSE);
	netMessageMap.RegisterNetMessage(server::RESOURCE_REQUEST);
	netMessageMap.RegisterNetMessage(server::RESOURCE_BEGIN);
	netMessageMap.RegisterNetMessage(server::QUERY_RESOURCE);
	netMessageMap.RegisterNetMessage(server::QUERY_MODEL_TEXTURE);

	netMessageMap.RegisterNetMessage(server::DISCONNECT);
	netMessageMap.RegisterNetMessage(server::USERINPUT);
	netMessageMap.RegisterNetMessage(server::CLIENTINFO);
	netMessageMap.RegisterNetMessage(server::GAME_READY);
	netMessageMap.RegisterNetMessage(server::CMD_CALL);
	netMessageMap.RegisterNetMessage(server::RCON);
	netMessageMap.RegisterNetMessage(server::SERVERINFO_REQUEST);
	netMessageMap.RegisterNetMessage(server::AUTHENTICATE);
	netMessageMap.RegisterNetMessage(server::LUANET);
	netMessageMap.RegisterNetMessage(server::CMD_SETPOS);
	netMessageMap.RegisterNetMessage(server::CVAR_SET);
	netMessageMap.RegisterNetMessage(server::NOCLIP);
	netMessageMap.RegisterNetMessage(server::NOTARGET);
	netMessageMap.RegisterNetMessage(server::GODMODE);
	netMessageMap.RegisterNetMessage(server::SUICIDE);
	netMessageMap.RegisterNetMessage(server::HURTME);
	netMessageMap.RegisterNetMessage(server::WEAPON_NEXT);
	netMessageMap.RegisterNetMessage(server::WEAPON_PREVIOUS);
	netMessageMap.RegisterNetMessage(server::ENT_EVENT);
	netMessageMap.RegisterNetMessage(server::GIVE_WEAPON);
	netMessageMap.RegisterNetMessage(server::STRIP_WEAPONS);
	netMessageMap.RegisterNetMessage(server::GIVE_AMMO);
	netMessageMap.RegisterNetMessage(server::DEBUG_AI_NAVIGATION);
	netMessageMap.RegisterNetMessage(server::DEBUG_AI_SCHEDULE_PRINT);
	netMessageMap.RegisterNetMessage(server::DEBUG_AI_SCHEDULE_TREE);

	netMessageMap.RegisterNetMessage(server::CL_SEND);
}
static void register_client_net_messages(pragma::networking::IBaseNetMessageMap &netMessageMap)
{
	using namespace pragma::networking::net_messages;
	netMessageMap.RegisterNetMessage(client::RESOURCEINFO);
	netMessageMap.RegisterNetMessage(client::RESOURCECOMPLETE);
	netMessageMap.RegisterNetMessage(client::RESOURCE_FRAGMENT);
	netMessageMap.RegisterNetMessage(client::RESOURCE_MDL_ROUGH);

	netMessageMap.RegisterNetMessage(client::SERVERINFO);
	netMessageMap.RegisterNetMessage(client::START_RESOURCE_TRANSFER);

	netMessageMap.RegisterNetMessage(client::SND_EV);
	netMessageMap.RegisterNetMessage(client::SND_CREATE);
	netMessageMap.RegisterNetMessage(client::SND_PRECACHE);

	netMessageMap.RegisterNetMessage(client::LUANET);
	netMessageMap.RegisterNetMessage(client::LUANET_REG);
	netMessageMap.RegisterNetMessage(client::REGISTER_NET_EVENT);

	netMessageMap.RegisterNetMessage(client::ENT_CREATE);
	netMessageMap.RegisterNetMessage(client::ENT_REMOVE);
	netMessageMap.RegisterNetMessage(client::ENT_SOUND);
	netMessageMap.RegisterNetMessage(client::ENT_MODEL);
	netMessageMap.RegisterNetMessage(client::ENT_SKIN);
	netMessageMap.RegisterNetMessage(client::PL_LOCAL);
	netMessageMap.RegisterNetMessage(client::GAME_START);
	netMessageMap.RegisterNetMessage(client::MAP_READY);
	netMessageMap.RegisterNetMessage(client::MAP_LOAD);
	netMessageMap.RegisterNetMessage(client::ENT_CREATE_LUA);
	netMessageMap.RegisterNetMessage(client::ENT_ANIM_PLAY);
	netMessageMap.RegisterNetMessage(client::ENT_MOVETYPE);
	netMessageMap.RegisterNetMessage(client::ENT_COLLISIONTYPE);
	netMessageMap.RegisterNetMessage(client::ENT_EYEOFFSET);
	netMessageMap.RegisterNetMessage(client::ENT_SETUNLIT);
	netMessageMap.RegisterNetMessage(client::ENT_SETCASTSHADOWS);
	netMessageMap.RegisterNetMessage(client::ENT_SETHEALTH);
	netMessageMap.RegisterNetMessage(client::ENT_SETNAME);
	netMessageMap.RegisterNetMessage(client::ENT_SETPARENT);
	netMessageMap.RegisterNetMessage(client::ENT_SETPARENTMODE);
	netMessageMap.RegisterNetMessage(client::ENT_PHYS_INIT);
	netMessageMap.RegisterNetMessage(client::ENT_PHYS_DESTROY);
	netMessageMap.RegisterNetMessage(client::ENT_EVENT);
	netMessageMap.RegisterNetMessage(client::ENT_TOGGLE);
	netMessageMap.RegisterNetMessage(client::ENT_SETCOLLISIONFILTER);
	netMessageMap.RegisterNetMessage(client::ENT_ANIM_GESTURE_PLAY);
	netMessageMap.RegisterNetMessage(client::ENT_ANIM_GESTURE_STOP);
	netMessageMap.RegisterNetMessage(client::ENT_SETKINEMATIC);
	netMessageMap.RegisterNetMessage(client::GAME_TIMESCALE);
	netMessageMap.RegisterNetMessage(client::FIRE_BULLET);
	netMessageMap.RegisterNetMessage(client::CREATE_GIBLET);
	netMessageMap.RegisterNetMessage(client::REGISTER_ENTITY_COMPONENT);
	netMessageMap.RegisterNetMessage(client::PL_TOGGLE_NOCLIP);

	netMessageMap.RegisterNetMessage(client::GAMEINFO);
	netMessageMap.RegisterNetMessage(client::GAME_READY);

	netMessageMap.RegisterNetMessage(client::SNAPSHOT);

	netMessageMap.RegisterNetMessage(client::CVAR_SET);
	netMessageMap.RegisterNetMessage(client::LUACMD_REG);

	netMessageMap.RegisterNetMessage(client::PLAYERINPUT);

	netMessageMap.RegisterNetMessage(client::PL_SPEED_WALK);
	netMessageMap.RegisterNetMessage(client::PL_SPEED_RUN);
	netMessageMap.RegisterNetMessage(client::PL_SPEED_SPRINT);
	netMessageMap.RegisterNetMessage(client::PL_SPEED_CROUCH_WALK);
	netMessageMap.RegisterNetMessage(client::PL_HEIGHT_STAND);
	netMessageMap.RegisterNetMessage(client::PL_HEIGHT_CROUCH);
	netMessageMap.RegisterNetMessage(client::PL_EYELEVEL_STAND);
	netMessageMap.RegisterNetMessage(client::PL_EYELEVEL_CROUCH);
	netMessageMap.RegisterNetMessage(client::PL_SLOPELIMIT);
	netMessageMap.RegisterNetMessage(client::PL_STEPOFFSET);
	netMessageMap.RegisterNetMessage(client::PL_UPDIRECTION);
	netMessageMap.RegisterNetMessage(client::PL_CHANGEDNAME);

	netMessageMap.RegisterNetMessage(client::WEP_DEPLOY);
	netMessageMap.RegisterNetMessage(client::WEP_HOLSTER);
	netMessageMap.RegisterNetMessage(client::WEP_PRIMARYATTACK);
	netMessageMap.RegisterNetMessage(client::WEP_SECONDARYATTACK);
	netMessageMap.RegisterNetMessage(client::WEP_ATTACK3);
	netMessageMap.RegisterNetMessage(client::WEP_ATTACK4);
	netMessageMap.RegisterNetMessage(client::WEP_RELOAD);
	netMessageMap.RegisterNetMessage(client::WEP_PRIM_CLIP_SIZE);
	netMessageMap.RegisterNetMessage(client::WEP_SEC_CLIP_SIZE);
	netMessageMap.RegisterNetMessage(client::WEP_PRIM_MAX_CLIP_SIZE);
	netMessageMap.RegisterNetMessage(client::WEP_SEC_MAX_CLIP_SIZE);
	netMessageMap.RegisterNetMessage(client::WEP_PRIM_AMMO_TYPE);
	netMessageMap.RegisterNetMessage(client::WEP_SEC_AMMO_TYPE);

	netMessageMap.RegisterNetMessage(client::ENV_LIGHT_SPOT_OUTERCUTOFF_ANGLE);
	netMessageMap.RegisterNetMessage(client::ENV_LIGHT_SPOT_INNERCUTOFF_ANGLE);

	netMessageMap.RegisterNetMessage(client::ENVLIGHT_SETSTATE);
	netMessageMap.RegisterNetMessage(client::ENVEXPLOSION_EXPLODE);

	netMessageMap.RegisterNetMessage(client::SV_SEND);

	netMessageMap.RegisterNetMessage(client::ENV_FOGCON_SETSTARTDIST);
	netMessageMap.RegisterNetMessage(client::ENV_FOGCON_SETENDDIST);
	netMessageMap.RegisterNetMessage(client::ENV_FOGCON_SETMAXDENSITY);

	netMessageMap.RegisterNetMessage(client::ENV_PRTSYS_SETCONTINUOUS);

	netMessageMap.RegisterNetMessage(client::CLIENT_DROPPED);
	netMessageMap.RegisterNetMessage(client::CLIENT_READY);
	netMessageMap.RegisterNetMessage(client::CLIENT_JOINED);

	netMessageMap.RegisterNetMessage(client::CREATE_EXPLOSION);
	netMessageMap.RegisterNetMessage(client::DEBUG_AI_NAVIGATION);
	netMessageMap.RegisterNetMessage(client::DEBUG_AI_SCHEDULE_PRINT);
	netMessageMap.RegisterNetMessage(client::DEBUG_AI_SCHEDULE_TREE);
	netMessageMap.RegisterNetMessage(client::CMD_CALL_RESPONSE);

	netMessageMap.RegisterNetMessage(client::ENT_TRIGGER_GRAVITY_ONSTARTTOUCH);

	netMessageMap.RegisterNetMessage(client::ADD_SHARED_COMPONENT);

	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWPOINT);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWLINE);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWBOX);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWTEXT);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWSPHERE);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWCONE);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWAXIS);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWPATH);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWSPLINE);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWPLANE);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAW_MESH);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWTRUNCATEDCONE);
	netMessageMap.RegisterNetMessage(client::DEBUG_DRAWCYLINDER);
}

void pragma::networking::register_net_messages()
{
	register_server_net_messages(*get_server_message_map());
	register_client_net_messages(*get_client_message_map());
}
