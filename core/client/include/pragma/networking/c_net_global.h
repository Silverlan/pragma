/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_NET_GLOBAL_H__
#define __C_NET_GLOBAL_H__
#include "pragma/networking/c_net_definitions.h"

DECLARE_NETMESSAGE_CL(serverinfo);
DECLARE_NETMESSAGE_CL(start_resource_transfer);

DECLARE_NETMESSAGE_CL(snd_ev);
DECLARE_NETMESSAGE_CL(snd_create);
DECLARE_NETMESSAGE_CL(snd_precache);

DECLARE_NETMESSAGE_CL(luanet);
DECLARE_NETMESSAGE_CL(luanet_reg);
DECLARE_NETMESSAGE_CL(register_net_event);

DECLARE_NETMESSAGE_CL(ent_create);
DECLARE_NETMESSAGE_CL(ent_remove);
DECLARE_NETMESSAGE_CL(ent_sound);
DECLARE_NETMESSAGE_CL(ent_model);
DECLARE_NETMESSAGE_CL(ent_skin);
DECLARE_NETMESSAGE_CL(pl_local);
DECLARE_NETMESSAGE_CL(game_start);
DECLARE_NETMESSAGE_CL(map_ready);
DECLARE_NETMESSAGE_CL(map_load);
DECLARE_NETMESSAGE_CL(ent_create_lua);
DECLARE_NETMESSAGE_CL(ent_anim_play);
DECLARE_NETMESSAGE_CL(ent_movetype);
DECLARE_NETMESSAGE_CL(ent_collisiontype);
DECLARE_NETMESSAGE_CL(ent_eyeoffset);
DECLARE_NETMESSAGE_CL(ent_setunlit);
DECLARE_NETMESSAGE_CL(ent_setcastshadows);
DECLARE_NETMESSAGE_CL(ent_sethealth);
DECLARE_NETMESSAGE_CL(ent_setname);
DECLARE_NETMESSAGE_CL(ent_setparent);
DECLARE_NETMESSAGE_CL(ent_setparentmode);
DECLARE_NETMESSAGE_CL(ent_phys_init);
DECLARE_NETMESSAGE_CL(ent_phys_destroy);
DECLARE_NETMESSAGE_CL(ent_event);
DECLARE_NETMESSAGE_CL(ent_toggle);
DECLARE_NETMESSAGE_CL(ent_setcollisionfilter);
DECLARE_NETMESSAGE_CL(ent_anim_gesture_play);
DECLARE_NETMESSAGE_CL(ent_anim_gesture_stop);
DECLARE_NETMESSAGE_CL(ent_setkinematic);
DECLARE_NETMESSAGE_CL(game_timescale);
DECLARE_NETMESSAGE_CL(fire_bullet);
DECLARE_NETMESSAGE_CL(create_giblet);
DECLARE_NETMESSAGE_CL(register_entity_component);
DECLARE_NETMESSAGE_CL(pl_toggle_noclip);

DECLARE_NETMESSAGE_CL(gameinfo);
DECLARE_NETMESSAGE_CL(game_ready);

DECLARE_NETMESSAGE_CL(snapshot);

DECLARE_NETMESSAGE_CL(cvar_set);
DECLARE_NETMESSAGE_CL(luacmd_reg);

DECLARE_NETMESSAGE_CL(playerinput);

DECLARE_NETMESSAGE_CL(pl_speed_walk);
DECLARE_NETMESSAGE_CL(pl_speed_run);
DECLARE_NETMESSAGE_CL(pl_speed_sprint);
DECLARE_NETMESSAGE_CL(pl_speed_crouch_walk);
DECLARE_NETMESSAGE_CL(pl_height_stand);
DECLARE_NETMESSAGE_CL(pl_height_crouch);
DECLARE_NETMESSAGE_CL(pl_eyelevel_stand);
DECLARE_NETMESSAGE_CL(pl_eyelevel_crouch);
DECLARE_NETMESSAGE_CL(pl_slopelimit);
DECLARE_NETMESSAGE_CL(pl_stepoffset);
DECLARE_NETMESSAGE_CL(pl_updirection);
DECLARE_NETMESSAGE_CL(pl_changedname);

DECLARE_NETMESSAGE_CL(wep_deploy);
DECLARE_NETMESSAGE_CL(wep_holster);
DECLARE_NETMESSAGE_CL(wep_primaryattack);
DECLARE_NETMESSAGE_CL(wep_secondaryattack);
DECLARE_NETMESSAGE_CL(wep_attack3);
DECLARE_NETMESSAGE_CL(wep_attack4);
DECLARE_NETMESSAGE_CL(wep_reload);
DECLARE_NETMESSAGE_CL(wep_prim_clip_size);
DECLARE_NETMESSAGE_CL(wep_sec_clip_size);
DECLARE_NETMESSAGE_CL(wep_prim_max_clip_size);
DECLARE_NETMESSAGE_CL(wep_sec_max_clip_size);
DECLARE_NETMESSAGE_CL(wep_prim_ammo_type);
DECLARE_NETMESSAGE_CL(wep_sec_ammo_type);

DECLARE_NETMESSAGE_CL(env_light_spot_outercutoff_angle);
DECLARE_NETMESSAGE_CL(env_light_spot_innercutoff_angle);

DECLARE_NETMESSAGE_CL(envlight_setstate);
DECLARE_NETMESSAGE_CL(envexplosion_explode);

DECLARE_NETMESSAGE_CL(sv_send);

DECLARE_NETMESSAGE_CL(env_fogcon_setstartdist);
DECLARE_NETMESSAGE_CL(env_fogcon_setenddist);
DECLARE_NETMESSAGE_CL(env_fogcon_setmaxdensity);

DECLARE_NETMESSAGE_CL(env_prtsys_setcontinuous);

DECLARE_NETMESSAGE_CL(client_dropped);
DECLARE_NETMESSAGE_CL(client_ready);
DECLARE_NETMESSAGE_CL(client_joined);

DECLARE_NETMESSAGE_CL(create_explosion);
DECLARE_NETMESSAGE_CL(debug_ai_navigation);
DECLARE_NETMESSAGE_CL(debug_ai_schedule_print);
DECLARE_NETMESSAGE_CL(debug_ai_schedule_tree);
DECLARE_NETMESSAGE_CL(cmd_call_response);

DECLARE_NETMESSAGE_CL(ent_trigger_gravity_onstarttouch);

DECLARE_NETMESSAGE_CL(add_shared_component);

#endif
