// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

DLLCLIENT void CMD_cl_send(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_cl_send_udp(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_cl_rcon(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_connect(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_disconnect(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
DLLCLIENT void CMD_cl_debug_netmessages(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
namespace {
	using namespace pragma::console::client;

	auto UVN = register_variable<udm::String>("playername", "player", ConVarFlags::Archive | ConVarFlags::Userinfo, "Local player name.");
	auto UVN = register_variable<udm::String>("password", "", ConVarFlags::Password, "Password which will be used for the next connection attempt.");
	auto UVN = register_variable<udm::UInt32>("cl_updaterate", 20, ConVarFlags::Archive, "The amount of times per second user input is being transmitted to the server.");
	auto UVN = register_variable<udm::Boolean>("net_graph", false, ConVarFlags::None, "Displays a graph about current network transmissions.");

	auto UVN = register_variable<udm::String>("cl_port_tcp", sci::DEFAULT_PORT_TCP, ConVarFlags::Archive | ConVarFlags::Userinfo, "Port used for TCP transmissions.");
	auto UVN = register_variable<udm::String>("cl_port_udp", sci::DEFAULT_PORT_UDP, ConVarFlags::Archive | ConVarFlags::Userinfo, "Port used for UDP transmissions.");

	auto UVN = register_variable<udm::Int32>("cl_max_fps", -1, ConVarFlags::Archive, "FPS will be clamped at this value. A value of < 0 deactivates the limit.");

	auto UVN = register_command("cl_send", &CMD_cl_send, ConVarFlags::None, "Sends a text message to the server and displays it in the console. Usage: cl_send <message>");
	auto UVN = register_command("cl_send_udp", &CMD_cl_send_udp, ConVarFlags::None, "Sends a text message to the server via UDP and displays it in the console. Usage: cl_send_udp <message>");
	auto UVN = register_command("rcon", &CMD_cl_rcon, ConVarFlags::None, "Sends the given command to the server and runs it serverside. Usage: rcon <command>");
	auto UVN = register_command("connect", &CMD_connect, ConVarFlags::None, "Attempts to connect to the specified server. Usage: connect <ipV6/ipV4> or connect <steamId> or without arguments to re-try the last attempt.");
	auto UVN = register_command("disconnect", &CMD_disconnect, ConVarFlags::None, "Disconnects from the server (if a connection is active), or closes the game if in single player mode.");
	auto UVN = register_command("cl_debug_netmessages", &CMD_cl_debug_netmessages, ConVarFlags::None, "Prints out debug information about recent net-messages.");
}
