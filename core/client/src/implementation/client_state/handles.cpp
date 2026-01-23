// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :client_state;
import :engine;
import :entities.components.world;
import :game;

void pragma::ClientState::HandlePacket(NetPacket &packet)
{
	packet->SetClient(true);
	CallCallbacks<void, std::reference_wrapper<NetPacket>>("OnReceivePacket", packet);
	unsigned int ID = packet.GetMessageID();
	networking::CLNetMessage *msg = GetNetMessage(ID);
	if(msg == nullptr) {
		Con::CWAR << "(CLIENT) Unhandled net message: " << ID << Con::endl;
		return;
	}
	// packet->SetClient(true); // WVTODO
	msg->handler(packet);
}

void pragma::ClientState::HandleConnect() { RequestServerInfo(); }

void pragma::ClientState::RequestServerInfo()
{
	Con::CCL << "Sending serverinfo request..." << Con::endl;
	NetPacket packet;
	packet->WriteString(GetConVarString("password"));
	SendPacket(networking::net_messages::server::SERVERINFO_REQUEST, packet, networking::Protocol::SlowReliable);
}

void pragma::ClientState::HandleClientReceiveServerInfo(NetPacket &packet)
{
	m_svInfo = std::make_unique<ServerInfo>();
	if(IsConnected()) {
		auto ip = m_client->GetIP();
		m_svInfo->address = ip.has_value() ? *ip : "unknown";
	}
	else
		m_svInfo->address = "[::1]";

	if(packet->Read<unsigned char>() == 1)
		m_svInfo->portUDP = packet->Read<unsigned short>();
	else
		m_svInfo->portUDP = 0;

	auto authRequired = packet->Read<bool>();
	NetPacket outAuthPacket {};
	outAuthPacket->Write<bool>(authRequired);
	if(authRequired) {
		// Authentication requires steamworks
		std::string err;
		auto libSteamworks = InitializeLibrary("steamworks/pr_steamworks", &err);
		if(libSteamworks == nullptr) {
			m_svInfo = nullptr;
			Con::CERR << "Unable to authenticate client: Steamworks module could not be loaded: " << err << Con::endl;
			Disconnect();
			return;
		}
		auto *fRequestAuthTicket = libSteamworks->FindSymbolAddress<bool (*)(std::vector<char> &, uint64_t &, std::shared_ptr<void> &)>("pr_steamworks_get_auth_session_ticket");
		uint64_t steamId;
		std::vector<char> token;
		std::shared_ptr<void> tokenHandle;
		if(fRequestAuthTicket == nullptr || fRequestAuthTicket(token, steamId, tokenHandle) == false) {
			m_svInfo = nullptr;
			Con::CERR << "Authentication failed! Disconnecting from server..." << Con::endl;
			Disconnect();
			return;
		}

		m_svInfo->authTokenHandle = tokenHandle;
		outAuthPacket->Write<uint64_t>(steamId);
		outAuthPacket->Write<uint16_t>(token.size());
		outAuthPacket->Write(reinterpret_cast<uint8_t *>(token.data()), token.size() * sizeof(token.front()));
	}
	SendPacket(networking::net_messages::server::AUTHENTICATE, outAuthPacket, networking::Protocol::SlowReliable);
}

void pragma::ClientState::HandleClientStartResourceTransfer(NetPacket &packet)
{
	if(m_svInfo == nullptr) {
		Disconnect();
		return;
	}
	auto svPath = m_svInfo->address;
	string::replace(svPath, ":", "_");
	string::replace(svPath, "[", "");
	string::replace(svPath, "]", "");
	m_svInfo->SetDownloadPath("downloads\\" + svPath + '\\');

	auto luaPath = m_svInfo->GetDownloadPath() + "lua";
	fs::create_path(luaPath);

	unsigned int numResources = packet->Read<unsigned int>();
	Con::CCL << "Downloading " << numResources << " files from server..." << Con::endl;

	StartResourceTransfer();
}

void pragma::ClientState::LoadLuaCache(std::string cache, unsigned int cacheSize)
{
	throw std::runtime_error {"Not implemented."};
#if 0
	std::string path = "cache\\" + cache + ".cache";
	auto f = pragma::fs::open_file(path.c_str(), pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary);
	if(f == nullptr) {
		Con::CWAR << "Unable to open lua-cache '" << cache << "' file!" << Con::endl;
		return;
	}
	unsigned int sourceLength = CUInt32(f->GetSize());
	int verbosity = 0;
	int small = 0;
	char *dest = new char[cacheSize];
	char *source = new char[sourceLength];
	f->Read(source, sourceLength);
	int err = BZ2_bzBuffToBuffDecompress(dest, &cacheSize, source, sourceLength, small, verbosity);
	if(err == BZ_OK) {
		unsigned int offset = 0;
		while(offset < cacheSize) {
			std::string path = pragma::string::read_until_etx(offset + dest);
			offset += CUInt32(path.size()) + 1;
			std::string content = pragma::string::read_until_etx(dest + offset);
			offset += CUInt32(content.length()) + 1;
			auto data = pragma::util::make_shared<std::vector<uint8_t>>();
			data->resize(content.length() + 1);
			memcpy(data->data(), content.c_str(), content.length() + 1);
			fs::AddVirtualFile((Lua::SCRIPT_DIRECTORY_SLASH + path).c_str(), data);
			//int s = luaL_dostring(m_lua,content.c_str());
			//lua_err(m_lua,s);
		}
	}
	else
		Con::CWAR << "Unable to decompress lua-cache (" << err << ")!" << Con::endl;
	delete[] dest;
	delete[] source;
#endif
}

extern pragma::ecs::CBaseEntity *NET_cl_ENT_CREATE(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false);
extern pragma::ecs::CBaseEntity *NET_cl_ENT_CREATE_LUA(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false);

void pragma::ClientState::ReadEntityData(NetPacket &packet)
{
	unsigned int numEnts = packet->Read<unsigned int>();
	std::vector<EntityHandle> ents;
	ents.reserve(numEnts);
	for(unsigned int i = 0; i < numEnts; i++) {
		auto bScripted = packet->Read<Bool>();
		ecs::CBaseEntity *ent = nullptr;
		if(bScripted == false)
			ent = NET_cl_ENT_CREATE(packet, false, true);
		else {
			auto offset = packet->GetOffset();
			auto entSize = packet->Read<UInt32>(); // Insurance, in case entity couldn't be created, or data hasn't been received properly
			ent = NET_cl_ENT_CREATE_LUA(packet, false, true);
			packet->SetOffset(offset + entSize);
		}
		if(ent != nullptr) {
			auto pMapComponent = ent->GetComponent<MapComponent>();
			if(pMapComponent.expired() || pMapComponent->GetMapIndex() == 0u)
				ents.push_back(ent->GetHandle());
		}
	} // Don't spawn them right away, in case they need to access each other
	for(unsigned int i = 0; i < ents.size(); i++) {
		EntityHandle &h = ents[i];
		if(h.valid())
			h->Spawn();
	}
}

void pragma::ClientState::HandleReceiveGameInfo(NetPacket &packet)
{
	if(IsGameActive())
		EndGame();
#ifdef DEBUG_SOCKET
	Con::CCL << "Received Game Information!" << Con::endl;
#endif
	// Read replicated ConVars
	auto numReplicated = packet->Read<uint32_t>();
	for(auto i = decltype(numReplicated) {0}; i < numReplicated; ++i) {
		auto id = packet->Read<uint32_t>();
		std::string cvar;
		auto bIdentifier = false;
		if(id == 0) {
			auto name = packet->ReadString();
			bIdentifier = true;
		}
		else
			bIdentifier = GetServerConVarIdentifier(id, cvar);

		auto value = packet->ReadString();
		if(bIdentifier == true) {
			auto *cf = GetConVar(cvar);
			if(cf != nullptr) {
				if(cf->GetType() == console::ConType::Var) {
					auto *cv = static_cast<console::ConVar *>(cf);
					if((cv->GetFlags() & console::ConVarFlags::Replicated) != console::ConVarFlags::None)
						SetConVar(cvar, value);
				}
			}
			else
				Con::CWAR << "Replicated ConVar " << cvar << " doesn't exist" << Con::endl;
		}
	}
	//
	game::GameModeManager::Initialize();
	//if(IsConnected())
	if(IsGameActive() == false)
		StartNewGame(GetConVarString("sv_gamemode"));
	auto *game = static_cast<CGame *>(GetGameState());
	game->InitializeGame();

	auto luaPath = m_svInfo->GetDownloadPath() + "lua";
	auto &scriptWatcher = m_game->GetLuaScriptWatcher();
	auto &resourceWatcher = m_game->GetResourceWatcher();

	scriptWatcher.MountDirectory(luaPath);
	resourceWatcher.MountDirectory("downloads\\");

	//if(game == nullptr)
	//	return;
	std::string map = packet->ReadString();
	auto svGameFlags = packet->Read<Game::GameFlags>();
	if((svGameFlags & Game::GameFlags::LevelTransition) != Game::GameFlags::None)
		game->SetGameFlags(game->GetGameFlags() | Game::GameFlags::LevelTransition);
	double tServer = packet->Read<double>();
	tServer -= game->CurTime();
	game->SetServerTime(tServer);

	std::vector<std::string> *msgs = game->GetLuaNetMessageIndices();
	msgs->clear();
	msgs->push_back("invalid");
	unsigned int numMessages = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numMessages; i++)
		msgs->push_back(packet->ReadString());

	auto &sharedNetEventIdToLocal = get_cgame()->GetSharedNetEventIdToLocal();
	sharedNetEventIdToLocal.clear();

	auto numEventIds = packet->Read<uint32_t>();
	sharedNetEventIdToLocal.resize(numEventIds, std::numeric_limits<NetEventId>::max());
	for(auto i = decltype(numEventIds) {0u}; i < numEventIds; ++i) {
		auto name = packet->ReadString();
		sharedNetEventIdToLocal[i] = get_cgame()->SetupNetEvent(name);
	}

	unsigned int numConCommands = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numConCommands; i++) {
		std::string scmd = packet->ReadString();
		unsigned int id = packet->Read<unsigned int>();
		RegisterServerConVar(scmd, id);
	}

	// Read component manager table
	auto &componentManager = static_cast<CEntityComponentManager &>(get_cgame()->GetEntityComponentManager());
	auto &componentTypes = componentManager.GetRegisteredComponentTypes();
	auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
	auto numTotalSvComponents = packet->Read<uint32_t>();
	auto numComponents = packet->Read<uint32_t>();
	svComponentToClComponentTable.resize(numTotalSvComponents, CEntityComponentManager::INVALID_COMPONENT);
	for(auto i = decltype(numComponents) {0u}; i < numComponents; ++i) {
		auto name = packet->ReadString();
		auto svId = packet->Read<ComponentId>();
		auto clComponentId = componentManager.PreRegisterComponentType(name);
		if(clComponentId >= svComponentToClComponentTable.size())
			svComponentToClComponentTable.resize(clComponentId + 1u, CEntityComponentManager::INVALID_COMPONENT);
		svComponentToClComponentTable.at(svId) = clComponentId;
	}
	//

	unsigned int cacheSize = packet->Read<unsigned int>();
	assert(cacheSize == 0);
	/*if(cacheSize > 0) {
		std::string cache = packet->ReadString();
		LoadLuaCache(cache, cacheSize);
	}*/
	game->SetUp();

	// Note: These have to be called BEFORE the map entities are created
	m_mapInfo = std::make_unique<MapInfo>();
	m_mapInfo->name = map;
	game->CallCallbacks("OnPreLoadMap");
	game->CallLuaCallbacks("OnPreLoadMap");

	ReadEntityData(packet);

	ecs::BaseEntity *wrld = networking::read_entity(packet);
	if(wrld != nullptr) {
		auto pWorldComponent = wrld->GetComponent<CWorldComponent>();
		game->SetWorld(pWorldComponent.get());
	}
	ChangeLevel(map.c_str());
	game->ReloadSoundCache();
}

void pragma::ClientState::SetGameReady()
{
	if(!m_game)
		return;
	SendPacket(networking::net_messages::server::GAME_READY, networking::Protocol::SlowReliable);
	m_game->OnGameReady();
}
