// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.server;

import :console.register_commands;
import pragma.shared;

static void CMD_entities_sv(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_map(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_list_maps(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_status_sv(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &);
static void CMD_drop(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_kick(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#ifdef _DEBUG
static void CMD_sv_dump_netmessages(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
#endif

static void CMD_sv_send(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv);
static void CMD_sv_send_udp(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv);
static void CMD_ent_input(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_ent_scale(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_ent_remove(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_ent_create(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_nav_reload(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_nav_generate(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &);
static void CMD_heartbeat(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &);
static void CMD_sv_debug_netmessages(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv);
static void CMD_startserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv);
static void CMD_closeserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &);

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

void CMD_drop(NetworkState *, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	if(pl == nullptr)
		return;
	auto sCharComponent = pl->GetEntity().GetComponent<pragma::SCharacterComponent>();
	if(sCharComponent.expired() == false)
		sCharComponent.get()->DropActiveWeapon();
}

void CMD_kick(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty() || argv.front().empty())
		return;
	pragma::SPlayerComponent *kickTarget = nullptr;
	auto &identifier = argv.front();
	auto name = identifier + "*";
	pragma::ecs::EntityIterator entIt {*SGame::Get()};
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
		Con::cwar << "No player with id or name '" << identifier << "' found!" << Con::endl;
		return;
	}
	std::string reason;
	if(argv.size() > 1)
		reason = argv[1];
	kickTarget->Kick(reason);
}

#ifdef _DEBUG
void CMD_sv_dump_netmessages(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto *map = GetServerMessageMap();
	std::unordered_map<std::string, unsigned int> *netmessages;
	map->GetNetMessages(&netmessages);
	if(!argv.empty()) {
		auto id = atoi(argv.front().c_str());
		for(auto it = netmessages->begin(); it != netmessages->end(); ++it) {
			if(it->second == id) {
				Con::cout << "Message Identifier: " << it->first << Con::endl;
				return;
			}
		}
		Con::cout << "No message with id " << id << " found!" << Con::endl;
		return;
	}
	for(auto it = netmessages->begin(); it != netmessages->end(); ++it)
		Con::cout << it->first << " = " << it->second << Con::endl;
}
#endif

void CMD_entities_sv(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!state->IsGameActive())
		return;
	auto sortedEnts = util::cmd::get_sorted_entities(*SGame::Get(), pl);
	std::optional<std::string> className = {};
	if(argv.empty() == false)
		className = '*' + argv.front() + '*';
	std::optional<std::string> modelName {};
	if(argv.size() > 1)
		modelName = '*' + argv[1] + '*';
	for(auto &pair : sortedEnts) {
		if(className.has_value() && ustring::match(pair.first->GetClass().c_str(), className->c_str()) == false)
			continue;
		if(modelName.has_value() && ustring::match(pair.first->GetModelName().c_str(), modelName->c_str()) == false)
			continue;
		Con::cout << *pair.first << Con::endl;
	}
}

void CMD_list_maps(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &)
{
	std::vector<std::string> files;
	auto exts = pragma::asset::get_supported_extensions(pragma::asset::Type::Map);
	for(auto &ext : exts)
		filemanager::find_files("maps/*." + ext, &files, nullptr);
	for(auto &f : files)
		ufile::remove_extension_from_filename(f, exts);
	std::sort(files.begin(), files.end());
	Con::cout << "Available maps:" << Con::endl;
	for(auto &f : files) {
		ufile::remove_extension_from_filename(f);
		Con::cout << f << Con::endl;
	}
	Con::cout << Con::endl;
}

void CMD_status_sv(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	auto &players = pragma::SPlayerComponent::GetAll();
	std::string ip;
	auto *sv = ServerState::Get()->GetServer();
	if(sv == nullptr) {
		std::stringstream str;
		str << "[::1]";
		ip = str.str();
	}
	else {
		auto hostIp = sv->GetHostIP();
		ip = hostIp.has_value() ? *hostIp : "Unknown";
	}
	auto &serverData = ServerState::Get()->GetServerData();
	Con::cout << "hostname:\t" << serverData.name << Con::endl;
	Con::cout << "udp/ip:\t\t" << ip << Con::endl;
	Con::cout << "map:\t\t" << serverData.map << Con::endl;
	Con::cout << "players:\t" << players.size() << " (" << serverData.maxPlayers << " max)" << Con::endl << Con::endl;
	Con::cout << "#  userid\tname    \tconnected\tping";
	auto bServerRunning = ServerState::Get()->IsServerRunning();
	if(bServerRunning == true)
		Con::cout << "\tadr";
	Con::cout << Con::endl;
	auto numPlayers = players.size();
	for(auto i = decltype(numPlayers) {0}; i < numPlayers; ++i) {
		auto *pl = players.at(i);
		auto *session = pl->GetClientSession();
		auto nameC = pl->GetEntity().GetNameComponent();
		Con::cout << "# \t" << i << "\t"
		          << "\"" << (nameC.valid() ? nameC->GetName() : "") << "\""
		          << "\t" << FormatTime(pl->TimeConnected()) << "     \t";
		if(session != nullptr)
			Con::cout << session->GetLatency();
		else
			Con::cout << "?";
		if(bServerRunning == true)
			Con::cout << "\t" << pl->GetClientIP();
		Con::cout << Con::endl;
	}
}

void CMD_sv_send(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		ServerState::Get()->SendPacket("sv_send", packet, pragma::networking::Protocol::SlowReliable);
	else {
		/*ServerState::Get()->
		ClientSession *cs = GetSessionByPlayerID(atoi(argv[0]));
		if(!cs)
		{
			Con::cout<<"No player with ID "<<atoi(argv[0])<<" found!"<<Con::endl;
			return;
		}
		ServerState::Get()->SendTCPMessage("sv_send",&packet,cs);*/
	}
}

void CMD_sv_send_udp(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	NetPacket packet;
	packet->WriteString(argv[(argv.size() == 1) ? 0 : 1]);
	if(argv.size() == 1)
		ServerState::Get()->SendPacket("sv_send", packet, pragma::networking::Protocol::FastUnreliable);
	else {
		/*ClientSession *cs = GetSessionByPlayerID(atoi(argv[0]));
		if(!cs)
		{
			Con::cout<<"No player with ID "<<atoi(argv[0])<<" found!"<<Con::endl;
			return;
		}
		ServerState::Get()->SendUDPMessage("sv_send",&packet,cs);*/
	}
}

void CMD_ent_input(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_input", state))
		return;
	if(SGame::Get() == nullptr)
		return;
	auto *activator = (pl != nullptr) ? &pl->GetEntity() : nullptr;
	if(argv.size() >= 2) {
		auto ents = command::find_named_targets(state, argv[0]);
		auto bFound = false;
		for(auto *ent : ents) {
			auto pInputComponent = ent->GetComponent<pragma::SIOComponent>();
			if(pInputComponent.valid()) {
				bFound = true;
				pInputComponent->Input(argv[1], activator, activator, "");
			}
		}
		if(bFound == false)
			Con::cwar << "No targets found by name '" << argv[0] << "'!" << Con::endl;
		return;
	}
	if(argv.size() < 1 || activator == nullptr || activator->IsCharacter() == false)
		return;
	auto charComponent = activator->GetCharacterComponent();
	auto ents = command::find_trace_targets(state, *charComponent);
	std::vector<std::string> substrings;
	ustring::explode_whitespace(argv.front(), substrings);
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
		Con::cwar << "No raycast target found!" << Con::endl;
}

void CMD_ent_scale(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_scale", state))
		return;
	if(SGame::Get() == nullptr)
		return;
	if(argv.size() >= 2) {
		auto ents = command::find_named_targets(state, argv[0]);
		auto scale = atof(argv[1].c_str());
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
	auto scale = atof(argv.front().c_str());
	auto ents = command::find_trace_targets(state, *charComponent);
	for(auto *ent : ents) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(pTransformComponent)
			pTransformComponent->SetScale(static_cast<float>(scale));
	}
}

void CMD_ent_remove(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_remove", state))
		return;
	if(SGame::Get() == nullptr || pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	if(ent.IsCharacter() == false)
		return;
	auto charComponent = ent.GetCharacterComponent();
	auto ents = command::find_target_entity(state, *charComponent, argv);
	if(ents.empty()) {
		Con::cwar << "No entity found to remove!" << Con::endl;
		return;
	}
	for(auto *ent : ents)
		ent->Remove();
}

void CMD_ent_create(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(!check_cheats("ent_create", state))
		return;
	if(SGame::Get() == nullptr)
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
	TraceData trData;
	trData.SetSource(origin);
	trData.SetTarget(origin + dir * static_cast<float>(pragma::GameLimits::MaxRayCastRange));
	trData.SetFilter(ent);
	trData.SetFlags(pragma::physics::RayCastFlags::Default | pragma::physics::RayCastFlags::InvertFilter | pragma::physics::RayCastFlags::IgnoreDynamic);
	auto r = SGame::Get()->RayCast(trData);
	if(r.hitType == pragma::physics::RayCastHitType::None) {
		Con::cwar << "No place to spawn entity!" << Con::endl;
		return;
	}
	std::string className = argv[0];
	pragma::ecs::BaseEntity *entNew = SGame::Get()->CreateEntity(className);
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

void CMD_nav_reload(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	if(SGame::Get() == nullptr)
		return;
	SGame::Get()->LoadNavMesh(true);
}

void CMD_nav_generate(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(SGame::Get() == nullptr)
		return;
	std::string map = SGame::Get()->GetMapName();
	if(map.empty())
		return;
	std::string err;
	Con::cout << "Generating navigation mesh..." << Con::endl;
	const pragma::nav::Config navCfg {
	  32.f /* walkableRadius */, 64.f, /* characterHeight */
	  20.f,                            /* maxClimbHeight */
	  45.f                             /* walkableSlopeAngle */
	};
	auto rcNavMesh = pragma::nav::generate(*SGame::Get(), navCfg, &err);
	if(rcNavMesh == nullptr)
		Con::cwar << "Unable to generate navigation mesh: " << err << Con::endl;
	else {
		auto navMesh = pragma::nav::Mesh::Create(rcNavMesh, navCfg);
		Con::cout << "Navigation mesh has been generated!" << Con::endl;
		std::string path = "maps\\" + map;
		path += "." + std::string {pragma::nav::PNAV_EXTENSION_BINARY};
		std::string err;
		if(navMesh->Save(*SGame::Get(), path, err) == false)
			Con::cwar << "Unable to save navigation mesh as '" << path << "': " << err << "!" << Con::endl;
	}
}

void CMD_heartbeat(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &)
{
	if(ServerState::Get() == nullptr)
		return;
	auto *sv = ServerState::Get()->GetServer();
	if(sv == nullptr)
		return;
	sv->Heartbeat();
}

void CMD_sv_debug_netmessages(NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *sv = ServerState::Get()->GetServer();
	if(sv == nullptr) {
		Con::cwar << "No server is active!" << Con::endl;
		return;
	}
	if(argv.size() > 0) {
		auto numBacklog = ustring::to_int(argv.front());
		sv->SetMemoryCount(numBacklog);
		Con::cout << "Debug backlog has been set to " << numBacklog << Con::endl;
		return;
	}
	auto *svMap = GetServerMessageMap();
	util::StringMap<uint32_t> *svMsgs;
	svMap->GetNetMessages(&svMsgs);

	auto *clMap = GetClientMessageMap();
	util::StringMap<uint32_t> *clMsgs;
	clMap->GetNetMessages(&clMsgs);

	sv->DebugPrint(*svMsgs, *clMsgs);
	sv->DebugDump("sv_netmessages.dump", *svMsgs, *clMsgs);
}

static void CMD_startserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &argv) { pragma::Engine::Get()->StartServer(false); }
static void CMD_closeserver(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &) { pragma::Engine::Get()->CloseServer(); }
