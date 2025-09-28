// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_CVAR_SETTINGS_H__
#define __S_CVAR_SETTINGS_H__

REGISTER_CONVAR_SV(sv_password, udm::Type::String, "", ConVarFlags::Archive, "Sets a password for the server. No player will be able to join, unless they provide the correct password using the 'password' command.");
REGISTER_CONVAR_SV(sv_maxplayers, udm::Type::UInt32, "1", ConVarFlags::Archive, "Specifies the maximum amount of players that are allowed to join the server.");

REGISTER_CONVAR_SV(sv_physics_simulation_enabled, udm::Type::Boolean, "1", ConVarFlags::Cheat, "Enables or disables physics simulation.");

REGISTER_CONVAR_SV(sv_water_surface_simulation_edge_iteration_count, udm::Type::UInt32, "5", ConVarFlags::Archive, "The more iterations, the more detailed the water simulation will be, but at a great performance cost.");
REGISTER_CONVAR_SV(sv_water_surface_simulation_shared, udm::Type::Boolean, "1", ConVarFlags::Archive, "If enabled, water surface simulation will be shared between client and server (Simulation is only performed once). This will only have an effect in single-player or on listen servers.");

#endif
