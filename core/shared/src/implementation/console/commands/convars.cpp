// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/console/helper.hpp"

module pragma.shared;

import :console;
import :engine;

void pragma::console::register_shared_convars(ConVarMap &map)
{
	map.RegisterConVar<udm::Boolean>("sv_cheats", false, ConVarFlags::Replicated | ConVarFlags::Notify, "Allows clients to run console commands which are marked as cheats if set to 1.");
	map.RegisterConVar<udm::Float>("sv_player_air_move_scale", 0.05, ConVarFlags::Replicated | ConVarFlags::Archive,
	  "The scale at which players are still able to move while in the air. 0 = Players can't move until they've landed, 1 = Players can move freely while in the air");
	map.RegisterConVar<udm::Float>("sv_noclip_speed", 200, ConVarFlags::Replicated | ConVarFlags::Archive, "The speed at which players can move in noclip mode.");
	map.RegisterConVar<udm::Boolean>("sv_allowcslua", true, ConVarFlags::Replicated | ConVarFlags::Notify | ConVarFlags::Archive, "1 = Clients can run own clientside scripts, 0 = Custom clientside scripts aren't allowed.");
	map.RegisterConVar<udm::Float>("host_timescale", 1, ConVarFlags::Replicated | ConVarFlags::Notify, "Specifies the timescale. A value of 2 means the game will run at twice the default speed.");
	map.RegisterConVar<udm::Vector3>("sv_gravity", Vector3(0, -600, 0), ConVarFlags::Replicated | ConVarFlags::Notify, "Specifies the default gravity force and direction. Certain entities may overwrite this for themselves.");
	map.RegisterConVar<udm::Float>("sv_friction", 1, ConVarFlags::Archive | ConVarFlags::Replicated | ConVarFlags::Notify, "Global friction scale.");
	map.RegisterConVar<udm::Float>("sv_restitution", 1, ConVarFlags::Archive | ConVarFlags::Replicated | ConVarFlags::Notify, "Global restitution scale.");
	map.RegisterConVar<udm::String>("sv_gamemode", "zen", ConVarFlags::Archive | ConVarFlags::Replicated, "Gamemode which should be used when starting a game.");
	map.RegisterConVar<udm::String>("sv_servername", engine_info::get_name(), ConVarFlags::Archive | ConVarFlags::Replicated, "The server name which will be displayed in the server browser.");
	map.RegisterConVar<udm::Float>("sv_timeout_duration", 90, ConVarFlags::Archive | ConVarFlags::Replicated, "Amount of time until a client is forcibly dropped if no data has been received.");
	map.RegisterConVar<udm::Float>("sv_tickrate", ENGINE_DEFAULT_TICK_RATE, ConVarFlags::Archive | ConVarFlags::Replicated,
	  "Specifies the tickrate. A higher tickrate means smoother and more reliable physics, but also more data to transmit to clients. Higher values can result in more lag for clients.");
	map.RegisterConVar<udm::Float>("sv_acceleration", 33, ConVarFlags::Archive | ConVarFlags::Replicated, "Player acceleration. If this is too low, the player will be unable to reach full movement speed due to friction forces.");
	map.RegisterConVar<udm::Float>("sv_acceleration_ramp_up_time", 0, ConVarFlags::Archive | ConVarFlags::Replicated, "The time it takes to reach full acceleration.");
}
