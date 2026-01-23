// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :console.commands;

static void CMD_cl_send(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_cl_send_udp(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_cl_rcon(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_connect(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_disconnect(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_cl_debug_netmessages(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
namespace {
	using namespace pragma::console::client;

	auto UVN = register_variable<udm::String>("playername", "player", pragma::console::ConVarFlags::Archive | pragma::console::ConVarFlags::Userinfo, "Local player name.");
	auto UVN = register_variable<udm::String>("password", "", pragma::console::ConVarFlags::Password, "Password which will be used for the next connection attempt.");
	auto UVN = register_variable<udm::UInt32>("cl_updaterate", 20, pragma::console::ConVarFlags::Archive, "The amount of times per second user input is being transmitted to the server.");
	auto UVN = register_variable<udm::Boolean>("net_graph", false, pragma::console::ConVarFlags::None, "Displays a graph about current network transmissions.");

#ifdef WINDOWS_CLANG_COMPILER_FIX
	auto UVN = register_variable<udm::String>("cl_port_tcp", "29150", pragma::console::ConVarFlags::Archive | pragma::console::ConVarFlags::Userinfo, "Port used for TCP transmissions.");
	auto UVN = register_variable<udm::String>("cl_port_udp", "29150", pragma::console::ConVarFlags::Archive | pragma::console::ConVarFlags::Userinfo, "Port used for UDP transmissions.");
#else
	auto UVN = register_variable<udm::String>("cl_port_tcp", pragma::networking::DEFAULT_PORT_TCP, pragma::console::ConVarFlags::Archive | pragma::console::ConVarFlags::Userinfo, "Port used for TCP transmissions.");
	auto UVN = register_variable<udm::String>("cl_port_udp", pragma::networking::DEFAULT_PORT_UDP, pragma::console::ConVarFlags::Archive | pragma::console::ConVarFlags::Userinfo, "Port used for UDP transmissions.");
#endif

	auto UVN = register_variable<udm::Int32>("cl_max_fps", -1, pragma::console::ConVarFlags::Archive, "FPS will be clamped at this value. A value of < 0 deactivates the limit.");

	auto UVN = register_command("cl_send", &CMD_cl_send, pragma::console::ConVarFlags::None, "Sends a text message to the server and displays it in the console. Usage: cl_send <message>");
	auto UVN = register_command("cl_send_udp", &CMD_cl_send_udp, pragma::console::ConVarFlags::None, "Sends a text message to the server via UDP and displays it in the console. Usage: cl_send_udp <message>");
	auto UVN = register_command("rcon", &CMD_cl_rcon, pragma::console::ConVarFlags::None, "Sends the given command to the server and runs it serverside. Usage: rcon <command>");
	auto UVN = register_command("connect", &CMD_connect, pragma::console::ConVarFlags::None, "Attempts to connect to the specified server. Usage: connect <ipV6/ipV4> or connect <steamId> or without arguments to re-try the last attempt.");
	auto UVN = register_command("disconnect", &CMD_disconnect, pragma::console::ConVarFlags::None, "Disconnects from the server (if a connection is active), or closes the game if in single player mode.");
	auto UVN = register_command("cl_debug_netmessages", &CMD_cl_debug_netmessages, pragma::console::ConVarFlags::None, "Prints out debug information about recent net-messages.");
}

///////////////////////////

void CMD_cl_rcon(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *client = pragma::get_client_state();
	if(!client->IsConnected() || argv.empty())
		return;
	std::string pass = pragma::get_cengine()->GetConVarString("rcon_password");
	NetPacket p;
	p->WriteString(pass);
	p->WriteString(argv[0]);
	client->SendPacket(pragma::networking::net_messages::server::RCON, p, pragma::networking::Protocol::SlowReliable);
}

void CMD_connect(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(argv.empty()) {
		auto &lastConnection = static_cast<pragma::ClientState *>(state)->GetLastConnectionInfo();
		if(lastConnection.address.has_value() == false && lastConnection.steamId.has_value() == false) {
			Con::COUT << "No previous connection attempt has been made! Please supply a destination address." << Con::endl;
			return;
		}
		if(lastConnection.address.has_value()) {
			std::vector<std::string> argv {lastConnection.address->first, std::to_string(lastConnection.address->second)};
			CMD_connect(state, pl, argv);
		}
		else if(lastConnection.steamId.has_value()) {
			std::vector<std::string> argv {std::to_string(*lastConnection.steamId)};
			CMD_connect(state, pl, argv);
		}
		return;
	}
	if(argv.size() == 1) {
		auto &address = argv[0];
		std::string ip;
		std::string port;
		pragma::string::remove_whitespace(address);
		if(!address.empty() && address[0] == '[') // IPv6
		{
			auto pos = address.find_first_of(']');
			if(pos != pragma::string::NOT_FOUND) {
				ip = address.substr(1, pos - 1);
				auto posPort = address.find_first_of(':', pos + 1);
				if(posPort != pragma::string::NOT_FOUND)
					port = address.substr(posPort + 1, address.length());
			}
			else
				ip = address;
		}
		else if(address.find('.') == std::string::npos) // SteamId
		{
			auto steamId = pragma::util::to_uint64(address);
			pragma::get_cengine()->Connect(steamId);
			return;
		}
		else {
			auto posPort = address.find_first_of(':');
			if(posPort != pragma::string::NOT_FOUND) {
				auto posIpv6 = address.find_last_of(':');
				if(posIpv6 != posPort) // IPv6
				{
					if(posIpv6 == (posPort + 1)) {
						std::vector<std::string> argv = {std::string("[") + address + std::string("]")};
						CMD_connect(state, pl, argv);
						return;
					}
					std::vector<std::string> argv = {address.substr(0, posIpv6), address.substr(posIpv6 + 1, address.length())};
					CMD_connect(state, pl, argv);
					return;
				}
				ip = address.substr(0, posPort);
				port = address.substr(posPort + 1, address.length());
			}
			else
				ip = address;
		}
		if(port.empty())
			port = pragma::networking::DEFAULT_PORT_TCP;
		pragma::get_cengine()->Connect(ip, port);
		return;
	}
	pragma::get_cengine()->Connect(argv[0], argv[1]);
}

void CMD_disconnect(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { pragma::get_cengine()->EndGame(); }

void CMD_cl_send(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[0]);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::CL_SEND, packet, pragma::networking::Protocol::SlowReliable);
}

void CMD_cl_send_udp(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[0]);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::CL_SEND, packet, pragma::networking::Protocol::FastUnreliable);
}

void CMD_cl_debug_netmessages(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *cl = pragma::get_client_state()->GetClient();
	if(cl == nullptr) {
		Con::CWAR << "No client is active!" << Con::endl;
		return;
	}
	if(argv.size() > 0) {
		auto numBacklog = pragma::string::to_int(argv.front());
		cl->SetMemoryCount(numBacklog);
		Con::COUT << "Debug backlog has been set to " << numBacklog << Con::endl;
		return;
	}
	auto *svMap = pragma::networking::get_server_message_map();
	pragma::util::StringMap<uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);

	auto *clMap = pragma::networking::get_client_message_map();
	pragma::util::StringMap<uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);

	cl->DebugPrint(*clMsgs, *svMsgs);
	cl->DebugDump("cl_netmessages.dump", *clMsgs, *svMsgs);
}
