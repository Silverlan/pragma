// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

////////////////////////////////////////////////////////////////////
////// Contains declarations for serverside convars / concmds //////
////// This is included in the SHARED library, NOT the server //////
///// ConCmd definitions have to be inside the server library //////
////////////////////////////////////////////////////////////////////
#ifdef ENGINE_NETWORK
#ifndef __S_CONVARS_H__
#define __S_CONVARS_H__
#include "pragma/networkdefinitions.h"
#include <pragma/console/convars.h>
#include "pragma/console/s_cvar_global.h"
#include "pragma/console/s_cvar_settings.h"
#include "pragma/console/s_cvar_server.h"
#include <pragma/engine.h>
#include <pragma/engine_info.hpp>

#ifndef STRING
#define STRING_HELPER(a) #a
#define STRING(a) STRING_HELPER(a)
#endif

REGISTER_SHARED_CONVAR(sv_cheats, udm::Type::Boolean, "0", ConVarFlags::Replicated | ConVarFlags::Notify, "Allows clients to run console commands which are marked as cheats if set to 1.");
REGISTER_SHARED_CONVAR(sv_player_air_move_scale, udm::Type::Float, "0.05", ConVarFlags::Replicated | ConVarFlags::Archive, "The scale at which players are still able to move while in the air. 0 = Players can't move until they've landed, 1 = Players can move freely while in the air");
REGISTER_SHARED_CONVAR(sv_noclip_speed, udm::Type::Float, "200", ConVarFlags::Replicated | ConVarFlags::Archive, "The speed at which players can move in noclip mode.");
REGISTER_SHARED_CONVAR(sv_allowcslua, udm::Type::Boolean, "1", ConVarFlags::Replicated | ConVarFlags::Notify | ConVarFlags::Archive, "1 = Clients can run own clientside scripts, 0 = Custom clientside scripts aren't allowed.");
REGISTER_SHARED_CONVAR(host_timescale, udm::Type::Float, "1", ConVarFlags::Replicated | ConVarFlags::Notify, "Specifies the timescale. A value of 2 means the game will run at twice the default speed.");
REGISTER_SHARED_CONVAR(sv_gravity, udm::Type::Vector3, "0 -600 0", ConVarFlags::Replicated | ConVarFlags::Notify, "Specifies the default gravity force and direction. Certain entities may overwrite this for themselves.");
REGISTER_SHARED_CONVAR(sv_friction, udm::Type::Float, "1", ConVarFlags::Archive | ConVarFlags::Replicated | ConVarFlags::Notify, "Global friction scale.");
REGISTER_SHARED_CONVAR(sv_restitution, udm::Type::Float, "1", ConVarFlags::Archive | ConVarFlags::Replicated | ConVarFlags::Notify, "Global restitution scale.");
REGISTER_SHARED_CONVAR(sv_gamemode, udm::Type::String, "zen", ConVarFlags::Archive | ConVarFlags::Replicated, "Gamemode which should be used when starting a game.");
REGISTER_SHARED_CONVAR(sv_servername, udm::Type::String, engine_info::get_name(), ConVarFlags::Archive | ConVarFlags::Replicated, "The server name which will be displayed in the server browser.");
REGISTER_SHARED_CONVAR(sv_timeout_duration, udm::Type::Float, "90", ConVarFlags::Archive | ConVarFlags::Replicated, "Amount of time until a client is forcibly dropped if no data has been received.");
REGISTER_SHARED_CONVAR(sv_tickrate, udm::Type::Float, STRING(ENGINE_DEFAULT_TICK_RATE), ConVarFlags::Archive | ConVarFlags::Replicated,
  "Specifies the tickrate. A higher tickrate means smoother and more reliable physics, but also more data to transmit to clients. Higher values can result in more lag for clients.");
REGISTER_SHARED_CONVAR(sv_acceleration, udm::Type::Float, "33", ConVarFlags::Archive | ConVarFlags::Replicated, "Player acceleration. If this is too low, the player will be unable to reach full movement speed due to friction forces.");
REGISTER_SHARED_CONVAR(sv_acceleration_ramp_up_time, udm::Type::Float, "0", ConVarFlags::Archive | ConVarFlags::Replicated, "The time it takes to reach full acceleration.");

REGISTER_CONVAR_SV(sv_allowdownload, udm::Type::Boolean, "1", ConVarFlags::Archive, "Specifies whether clients are allowed to download resources from the server.");
REGISTER_CONVAR_SV(sv_allowupload, udm::Type::Boolean, "1", ConVarFlags::Archive, "Specifies whether clients are allowed to upload resources to the server (e.g. spraylogos).");
#endif
#endif
