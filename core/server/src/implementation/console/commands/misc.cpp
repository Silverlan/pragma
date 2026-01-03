// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.server;

import :console.register_commands;
import pragma.shared;

static void CMD_entities_sv(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_map(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_list_maps(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_status_sv(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_drop(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_kick(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#ifdef _DEBUG
static void CMD_sv_dump_netmessages(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#endif

static void CMD_sv_send(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv);
static void CMD_sv_send_udp(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv);
static void CMD_ent_input(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_ent_scale(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_ent_remove(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_ent_create(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_nav_reload(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_nav_generate(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &);
static void CMD_heartbeat(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &);
static void CMD_sv_debug_netmessages(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_startserver(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv);
static void CMD_closeserver(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &);

namespace {
	using namespace pragma::console::server;
	auto UVN = register_command("entities", &CMD_entities_sv, pragma::console::ConVarFlags::None, "Prints a list of all current serverside entities in the world.");
	auto UVN = register_command("list_maps", &CMD_list_maps, pragma::console::ConVarFlags::None, "Prints a list of all available list to the console.");
	auto UVN = register_command("status", &CMD_status_sv, pragma::console::ConVarFlags::None, "Prints information about the server to the console.");
	auto UVN = register_command("drop", &CMD_drop, pragma::console::ConVarFlags::None, "Drops the player's active weapon.");
	auto UVN = register_command("kick", &CMD_kick, pragma::console::ConVarFlags::None, "Kicks the specified player for the given reason. Usage: kick <playerId/playerName> <reason>");
#ifdef _DEBUG
	auto UVN = register_command("sv_dump_netmessages", &CMD_sv_dump_netmessages, pragma::console::ConVarFlags::None, "Prints all registered netmessages to the console.");
#endif

	auto UVN = register_command("sv_send", &CMD_sv_send, pragma::console::ConVarFlags::None, "Sends a text message to all connected clients and displays it in the console. Usage: sv_send <message>");
	auto UVN = register_command("sv_send_udp", &CMD_sv_send_udp, pragma::console::ConVarFlags::None, "Sends a text message to all connected clients via UDP and displays it in the console. Usage: sv_send_udp <message>");
	auto UVN = register_command("startserver", &CMD_startserver, pragma::console::ConVarFlags::None, "Starts an internet server. Requires a running game.");
	auto UVN = register_command("closeserver", &CMD_closeserver, pragma::console::ConVarFlags::None, "Closes the server (if active) and drops all connected clients.");
	auto UVN = register_command("ent_input", &CMD_ent_input, pragma::console::ConVarFlags::None, "Triggers the given input on the specified entity. Usage: ent_input <entityName/entityClass> <input>");
	auto UVN = register_command("ent_scale", &CMD_ent_scale, pragma::console::ConVarFlags::None, "Changes the scale of the specified entity. Usage: ent_input <entityName/entityClass> <scale>");
	auto UVN = register_command("ent_remove", &CMD_ent_remove, pragma::console::ConVarFlags::None, "Removes the specified entity, or the entity the player is looking at if no argument is provided. Usage: ent_remove <entityName/className>");
	auto UVN = register_command("ent_create", &CMD_ent_create, pragma::console::ConVarFlags::None, "Creates and spawns a new entity with the given class name at the position in the world the player is looking at. Usage: ent_create <className>");
	auto UVN = register_command("nav_generate", &CMD_nav_generate, pragma::console::ConVarFlags::None, "Generates a navigation mesh for the current map and saves it as a navigation file.");
	auto UVN = register_command("nav_reload", &CMD_nav_reload, pragma::console::ConVarFlags::None, "Reloads the navigation mesh for the current map.");
	auto UVN = register_command("heartbeat", &CMD_heartbeat, pragma::console::ConVarFlags::None, "Instantly sends a heartbeat to the master server.");
	auto UVN = register_command("sv_debug_netmessages", &CMD_sv_debug_netmessages, pragma::console::ConVarFlags::None, "Prints out debug information about recent net-messages.");
}

void CMD_drop(pragma::NetworkState *, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	if(pl == nullptr)
		return;
	auto sCharComponent = pl->GetEntity().GetComponent<pragma::SCharacterComponent>();
	if(sCharComponent.expired() == false)
		sCharComponent.get()->DropActiveWeapon();
}

void CMD_kick(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty() || argv.front().empty())
		return;
	pragma::SPlayerComponent *kickTarget = nullptr;
	auto &identifier = argv.front();
	auto name = identifier + "*";
	pragma::ecs::EntityIterator entIt {*pragma::SGame::Get()};
	entIt.AttachFilter<EntityIteratorFilterName>(name, false, false);
	auto it = entIt.begin();
	auto *ent = (it != entIt.end()) ? *it : nullptr;
	auto plComponent = (ent != nullptr) ? ent->GetComponent<pragma::SPlayerComponent>() : pragma::ComponentHandle<pragma::SPlayerComponent> {};
	if(plComponent.expired() == false)
		kickTarget = plComponent.get();
	else {
		char *p;
		auto id = strtol(identifier.c_str(), &p, 10);
		if(p != nullptr) {
			auto &players = pragma::SPlayerComponent::GetAll();
			if(id < players.size() && id >= 0)
				kickTarget = players.at(id);
		}
	}
	if(kickTarget == nullptr) {
		Con::CWAR << "No player with id or name '" << identifier << "' found!" << Con::endl;
		return;
	}
	std::string reason;
	if(argv.size() > 1)
		reason = argv[1];
	kickTarget->Kick(reason);
}

#ifdef _DEBUG
void CMD_sv_dump_netmessages(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *map = GetServerMessageMap();
	std::unordered_map<std::string, unsigned int> *netmessages;
	map->GetNetMessages(&netmessages);
	if(!argv.empty()) {
		auto id = pragma::string::to_int(argv.front());
		for(auto it = netmessages->begin(); it != netmessages->end(); ++it) {
			if(it->second == id) {
				Con::COUT << "Message Identifier: " << it->first << Con::endl;
				return;
			}
		}
		Con::COUT << "No message with id " << id << " found!" << Con::endl;
		return;
	}
	for(auto it = netmessages->begin(); it != netmessages->end(); ++it)
		Con::COUT << it->first << " = " << it->second << Con::endl;
}
#endif

void CMD_entities_sv(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!state->IsGameActive())
		return;
	auto sortedEnts = pragma::console::get_sorted_entities(*pragma::SGame::Get(), pl);
	std::optional<std::string> className = {};
	if(argv.empty() == false)
		className = '*' + argv.front() + '*';
	std::optional<std::string> modelName {};
	if(argv.size() > 1)
		modelName = '*' + argv[1] + '*';
	for(auto &pair : sortedEnts) {
		if(className.has_value() && pragma::string::match(pair.first->GetClass().c_str(), className->c_str()) == false)
			continue;
		if(modelName.has_value() && pragma::string::match(pair.first->GetModelName().c_str(), modelName->c_str()) == false)
			continue;
		Con::COUT << *pair.first << Con::endl;
	}
}

void CMD_list_maps(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	std::vector<std::string> files;
	auto exts = pragma::asset::get_supported_extensions(pragma::asset::Type::Map);
	for(auto &ext : exts)
		pragma::fs::find_files("maps/*." + ext, &files, nullptr);
	for(auto &f : files)
		ufile::remove_extension_from_filename(f, exts);
	std::sort(files.begin(), files.end());
	Con::COUT << "Available maps:" << Con::endl;
	for(auto &f : files) {
		ufile::remove_extension_from_filename(f);
		Con::COUT << f << Con::endl;
	}
	Con::COUT << Con::endl;
}

void CMD_status_sv(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &players = pragma::SPlayerComponent::GetAll();
	std::string ip;
	auto *sv = pragma::ServerState::Get()->GetServer();
	if(sv == nullptr) {
		std::stringstream str;
		str << "[::1]";
		ip = str.str();
	}
	else {
		auto hostIp = sv->GetHostIP();
		ip = hostIp.has_value() ? *hostIp : "Unknown";
	}
	auto &serverData = pragma::ServerState::Get()->GetServerData();
	Con::COUT << "hostname:\t" << serverData.name << Con::endl;
	Con::COUT << "udp/ip:\t\t" << ip << Con::endl;
	Con::COUT << "map:\t\t" << serverData.map << Con::endl;
	Con::COUT << "players:\t" << players.size() << " (" << serverData.maxPlayers << " max)" << Con::endl << Con::endl;
	Con::COUT << "#  userid\tname    \tconnected\tping";
	auto bServerRunning = pragma::ServerState::Get()->IsServerRunning();
	if(bServerRunning == true)
		Con::COUT << "\tadr";
	Con::COUT << Con::endl;
	auto numPlayers = players.size();
	for(auto i = decltype(numPlayers) {0}; i < numPlayers; ++i) {
		auto *pl = players.at(i);
		auto *session = pl->GetClientSession();
		auto nameC = pl->GetEntity().GetNameComponent();
		Con::COUT << "# \t" << i << "\t"
		          << "\"" << (nameC.valid() ? nameC->GetName() : "") << "\""
		          << "\t" << pragma::string::format_time(pl->TimeConnected()) << "     \t";
		if(session != nullptr)
			Con::COUT << session->GetLatency();
		else
			Con::COUT << "?";
		if(bServerRunning == true)
			Con::COUT << "\t" << pl->GetClientIP();
		Con::COUT << Con::endl;
	}
}

void CMD_sv_send(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::SV_SEND, packet, pragma::networking::Protocol::SlowReliable);
	else {
		/*ServerState::Get()->
		ClientSession *cs = GetSessionByPlayerID(pragma::string::to_int(argv[0]));
		if(!cs)
		{
			Con::COUT<<"No player with ID "<<pragma::string::to_int(argv[0])<<" found!"<<Con::endl;
			return;
		}
		ServerState::Get()->SendTCPMessage(pragma::networking::net_messages::client::SV_SEND, &packet,cs);*/
	}
}

void CMD_sv_send_udp(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		pragma::ServerState::Get()->SendPacket(pragma::networking::net_messages::client::SV_SEND, packet, pragma::networking::Protocol::FastUnreliable);
	else {
		/*ClientSession *cs = GetSessionByPlayerID(pragma::string::to_int(argv[0]));
		if(!cs)
		{
			Con::COUT<<"No player with ID "<<pragma::string::to_int(argv[0])<<" found!"<<Con::endl;
			return;
		}
		ServerState::Get()->SendUDPMessage(pragma::networking::net_messages::client::SV_SEND,&packet,cs);*/
	}
}

void CMD_ent_input(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_input", state))
		return;
	if(pragma::SGame::Get() == nullptr)
		return;
	auto *activator = (pl != nullptr) ? &pl->GetEntity() : nullptr;
	if(argv.size() >= 2) {
		auto ents = pragma::console::find_named_targets(state, argv[0]);
		auto bFound = false;
		for(auto *ent : ents) {
			auto pInputComponent = ent->GetComponent<pragma::SIOComponent>();
			if(pInputComponent.valid()) {
				bFound = true;
				pInputComponent->Input(argv[1], activator, activator, "");
			}
		}
		if(bFound == false)
			Con::CWAR << "No targets found by name '" << argv[0] << "'!" << Con::endl;
		return;
	}
	if(argv.size() < 1 || activator == nullptr || activator->IsCharacter() == false)
		return;
	auto charComponent = activator->GetCharacterComponent();
	auto ents = pragma::console::find_trace_targets(state, *charComponent);
	std::vector<std::string> substrings;
	pragma::string::explode_whitespace(argv.front(), substrings);
	if(substrings.empty() == true)
		return;
	auto input = substrings.front();
	auto data = (substrings.size() > 1) ? substrings.at(1) : "";
	auto bFound = false;
	for(auto *ent : ents) {
		auto pInputComponent = ent->GetComponent<pragma::SIOComponent>();
		if(pInputComponent.valid()) {
			bFound = true;
			pInputComponent->Input(input, activator, activator, data);
		}
	}
	if(bFound == false)
		Con::CWAR << "No raycast target found!" << Con::endl;
}

void CMD_ent_scale(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_scale", state))
		return;
	if(pragma::SGame::Get() == nullptr)
		return;
	if(argv.size() >= 2) {
		auto ents = pragma::console::find_named_targets(state, argv[0]);
		auto scale = pragma::string::to_float(argv[1]);
		for(auto *ent : ents) {
			auto pTransformComponent = ent->GetTransformComponent();
			if(pTransformComponent)
				pTransformComponent->SetScale(static_cast<float>(scale));
		}
		return;
	}
	if(argv.size() < 1 || pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto scale = pragma::string::to_float(argv.front());
	auto ents = pragma::console::find_trace_targets(state, *charComponent);
	for(auto *ent : ents) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(pTransformComponent)
			pTransformComponent->SetScale(static_cast<float>(scale));
	}
}

void CMD_ent_remove(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_remove", state))
		return;
	if(pragma::SGame::Get() == nullptr || pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = pragma::console::find_target_entity(state, *charComponent, argv);
	if(ents.empty()) {
		Con::CWAR << "No entity found to remove!" << Con::endl;
		return;
	}
	for(auto *ent : ents)
		ent->Remove();
}

void CMD_ent_create(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_create", state))
		return;
	if(pragma::SGame::Get() == nullptr)
		return;
	if(argv.empty() || pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(pTrComponent == nullptr)
		return;
	auto charComponent = ent.GetCharacterComponent();
	Vector3 origin = pTrComponent->GetEyePosition();
	Vector3 dir = charComponent.valid() ? charComponent->GetViewForward() : pTrComponent->GetForward();
	pragma::physics::TraceData trData;
	trData.SetSource(origin);
	trData.SetTarget(origin + dir * static_cast<float>(pragma::GameLimits::MaxRayCastRange));
	trData.SetFilter(ent);
	trData.SetFlags(pragma::physics::RayCastFlags::Default | pragma::physics::RayCastFlags::InvertFilter | pragma::physics::RayCastFlags::IgnoreDynamic);
	auto r = pragma::SGame::Get()->RayCast(trData);
	if(r.hitType == pragma::physics::RayCastHitType::None) {
		Con::CWAR << "No place to spawn entity!" << Con::endl;
		return;
	}
	std::string className = argv[0];
	pragma::ecs::BaseEntity *entNew = pragma::SGame::Get()->CreateEntity(className);
	if(entNew == nullptr)
		return;
	auto pTrComponentNew = entNew->GetTransformComponent();
	if(pTrComponentNew) {
		auto posSpawn = r.position;
		auto pPhysComponent = entNew->GetPhysicsComponent();
		if(pPhysComponent != nullptr)
			posSpawn += r.normal * pPhysComponent->GetCollisionRadius();
		else
			posSpawn += r.normal * 1.f;
		pTrComponentNew->SetPosition(posSpawn);
	}
	if(argv.size() > 1) {
		auto pNameComponent = entNew->GetComponent<pragma::SNameComponent>();
		if(pNameComponent.valid())
			pNameComponent->SetName(argv[1]);
	}
	entNew->Spawn();
}

void CMD_nav_reload(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(pragma::SGame::Get() == nullptr)
		return;
	pragma::SGame::Get()->LoadNavMesh(true);
}

void CMD_nav_generate(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(pragma::SGame::Get() == nullptr)
		return;
	std::string map = pragma::SGame::Get()->GetMapName();
	if(map.empty())
		return;
	std::string err;
	Con::COUT << "Generating navigation mesh..." << Con::endl;
	const pragma::nav::Config navCfg {
	  32.f /* walkableRadius */, 64.f, /* characterHeight */
	  20.f,                            /* maxClimbHeight */
	  45.f                             /* walkableSlopeAngle */
	};
	auto rcNavMesh = pragma::nav::generate(*pragma::SGame::Get(), navCfg, &err);
	if(rcNavMesh == nullptr)
		Con::CWAR << "Unable to generate navigation mesh: " << err << Con::endl;
	else {
		auto navMesh = pragma::nav::Mesh::Create(rcNavMesh, navCfg);
		Con::COUT << "Navigation mesh has been generated!" << Con::endl;
		std::string path = "maps\\" + map;
		path += "." + std::string {pragma::nav::PNAV_EXTENSION_BINARY};
		std::string err;
		if(navMesh->Save(*pragma::SGame::Get(), path, err) == false)
			Con::CWAR << "Unable to save navigation mesh as '" << path << "': " << err << "!" << Con::endl;
	}
}

void CMD_heartbeat(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(pragma::ServerState::Get() == nullptr)
		return;
	auto *sv = pragma::ServerState::Get()->GetServer();
	if(sv == nullptr)
		return;
	sv->Heartbeat();
}

void CMD_sv_debug_netmessages(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *sv = pragma::ServerState::Get()->GetServer();
	if(sv == nullptr) {
		Con::CWAR << "No server is active!" << Con::endl;
		return;
	}
	if(argv.size() > 0) {
		auto numBacklog = pragma::string::to_int(argv.front());
		sv->SetMemoryCount(numBacklog);
		Con::COUT << "Debug backlog has been set to " << numBacklog << Con::endl;
		return;
	}
	auto *svMap = pragma::networking::get_server_message_map();
	pragma::util::StringMap<uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);

	auto *clMap = pragma::networking::get_client_message_map();
	pragma::util::StringMap<uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);

	sv->DebugPrint(*svMsgs, *clMsgs);
	sv->DebugDump("sv_netmessages.dump", *svMsgs, *clMsgs);
}

static void CMD_startserver(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv) { pragma::Engine::Get()->StartServer(false); }
static void CMD_closeserver(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { pragma::Engine::Get()->CloseServer(); }
