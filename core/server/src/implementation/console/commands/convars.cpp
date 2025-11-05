// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.server;
import :console.register_commands;

namespace {
	using namespace pragma::console::server;
	auto UVN = register_variable<udm::String>("sv_password", "", pragma::console::ConVarFlags::Archive, "Sets a password for the server. No player will be able to join, unless they provide the correct password using the 'password' command.");
	auto UVN = register_variable<udm::UInt32>("sv_maxplayers", 1, pragma::console::ConVarFlags::Archive, "Specifies the maximum amount of players that are allowed to join the server.");
	auto UVN = register_variable<udm::Boolean>("sv_physics_simulation_enabled", true, pragma::console::ConVarFlags::Cheat, "Enables or disables physics simulation.");
	auto UVN = register_variable<udm::UInt32>("sv_water_surface_simulation_edge_iteration_count", 5, pragma::console::ConVarFlags::Archive, "The more iterations, the more detailed the water simulation will be, but at a great performance cost.");
	auto UVN = register_variable<udm::Boolean>("sv_water_surface_simulation_shared", true, pragma::console::ConVarFlags::Archive,
	  "If enabled, water surface simulation will be shared between client and server (Simulation is only performed once). This will only have an effect in single-player or on listen servers.");

	auto UVN = register_variable<udm::Boolean>("sv_allowdownload", true, pragma::console::ConVarFlags::Archive, "Specifies whether clients are allowed to download resources from the server.");
	auto UVN = register_variable<udm::Boolean>("sv_allowupload", true, pragma::console::ConVarFlags::Archive, "Specifies whether clients are allowed to upload resources to the server (e.g. spraylogos).");

	auto UVN = register_variable<udm::String>("sv_port_tcp", "29150", pragma::console::ConVarFlags::Archive, "TCP port which will be used when starting a server.");
	auto UVN = register_variable<udm::String>("sv_allowdownload", "29150", pragma::console::ConVarFlags::Archive, "UDP port which will be used when starting a server.");
	auto UVN = register_variable<udm::Boolean>("sv_use_p2p_if_available", true, pragma::console::ConVarFlags::Archive, "Use a peer-to-peer connection if the selected networking layer supports it.");
}
