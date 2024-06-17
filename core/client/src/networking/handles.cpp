/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientutil.h"
#include <fsys/filesystem.h>
#include "pragma/c_engine.h"
#include "pragma/networking/netmessages.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/networking/iclient.hpp"
#include <sharedutils/util_string.h>
#include <sharedutils/util_library.hpp>
#include "pragma/entities/components/c_entity_component.hpp"
#include "pragma/entities/c_world.h"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/resources.h>
#include <pragma/lua/lua_script_watcher.h>
#include <pragma/util/resource_watcher.h>
#include <pragma/game/gamemode/gamemodemanager.h>
#include <pragma/entities/components/map_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

void ClientState::HandlePacket(NetPacket &packet)
{
	packet->SetClient(true);
	CallCallbacks<void, std::reference_wrapper<NetPacket>>("OnReceivePacket", packet);
	unsigned int ID = packet.GetMessageID();
	CLNetMessage *msg = GetNetMessage(ID);
	if(msg == NULL) {
		Con::cwar << "(CLIENT) Unhandled net message: " << ID << Con::endl;
		return;
	}
	// packet->SetClient(true); // WVTODO
	msg->handler(packet);
}

void ClientState::HandleConnect() { RequestServerInfo(); }

void ClientState::RequestServerInfo()
{
	Con::ccl << "Sending serverinfo request..." << Con::endl;
	NetPacket packet;
	packet->WriteString(GetConVarString("password"));
	SendPacket("serverinfo_request", packet, pragma::networking::Protocol::SlowReliable);
}

void ClientState::HandleClientReceiveServerInfo(NetPacket &packet)
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
			Con::cerr << "Unable to authenticate client: Steamworks module could not be loaded: " << err << Con::endl;
			Disconnect();
			return;
		}
		auto *fRequestAuthTicket = libSteamworks->FindSymbolAddress<bool (*)(std::vector<char> &, uint64_t &, std::shared_ptr<void> &)>("pr_steamworks_get_auth_session_ticket");
		uint64_t steamId;
		std::vector<char> token;
		std::shared_ptr<void> tokenHandle;
		if(fRequestAuthTicket == nullptr || fRequestAuthTicket(token, steamId, tokenHandle) == false) {
			m_svInfo = nullptr;
			Con::cerr << "Authentication failed! Disconnecting from server..." << Con::endl;
			Disconnect();
			return;
		}

		m_svInfo->authTokenHandle = tokenHandle;
		outAuthPacket->Write<uint64_t>(steamId);
		outAuthPacket->Write<uint16_t>(token.size());
		outAuthPacket->Write(reinterpret_cast<uint8_t *>(token.data()), token.size() * sizeof(token.front()));
	}
	SendPacket("authenticate", outAuthPacket, pragma::networking::Protocol::SlowReliable);
}

void ClientState::HandleClientStartResourceTransfer(NetPacket &packet)
{
	if(m_svInfo == nullptr) {
		Disconnect();
		return;
	}
	auto svPath = m_svInfo->address;
	ustring::replace(svPath, ":", "_");
	ustring::replace(svPath, "[", "");
	ustring::replace(svPath, "]", "");
	m_svInfo->SetDownloadPath("downloads\\" + svPath + '\\');

	auto luaPath = m_svInfo->GetDownloadPath() + "lua";
	FileManager::CreatePath(luaPath.c_str());

	unsigned int numResources = packet->Read<unsigned int>();
	Con::ccl << "Downloading " << numResources << " files from server..." << Con::endl;

	StartResourceTransfer();
}

void ClientState::LoadLuaCache(std::string cache, unsigned int cacheSize)
{
	throw std::runtime_error {"Not implemented."};
#if 0
	std::string path = "cache\\" + cache + ".cache";
	auto f = FileManager::OpenFile(path.c_str(), "rb");
	if(f == NULL) {
		Con::cwar << "Unable to open lua-cache '" << cache << "' file!" << Con::endl;
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
			std::string path = ustring::read_until_etx(offset + dest);
			offset += CUInt32(path.size()) + 1;
			std::string content = ustring::read_until_etx(dest + offset);
			offset += CUInt32(content.length()) + 1;
			auto data = std::make_shared<std::vector<uint8_t>>();
			data->resize(content.length() + 1);
			memcpy(data->data(), content.c_str(), content.length() + 1);
			FileManager::AddVirtualFile((Lua::SCRIPT_DIRECTORY_SLASH + path).c_str(), data);
			//int s = luaL_dostring(m_lua,content.c_str());
			//lua_err(m_lua,s);
		}
	}
	else
		Con::cwar << "Unable to decompress lua-cache (" << err << ")!" << Con::endl;
	delete[] dest;
	delete[] source;
#endif
}

extern CBaseEntity *NET_cl_ent_create(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false);
extern CBaseEntity *NET_cl_ent_create_lua(NetPacket &packet, bool bSpawn, bool bIgnoreMapInit = false);

void ClientState::ReadEntityData(NetPacket &packet)
{
	unsigned int numEnts = packet->Read<unsigned int>();
	std::vector<EntityHandle> ents;
	ents.reserve(numEnts);
	for(unsigned int i = 0; i < numEnts; i++) {
		auto bScripted = packet->Read<Bool>();
		CBaseEntity *ent = nullptr;
		if(bScripted == false)
			ent = NET_cl_ent_create(packet, false, true);
		else {
			auto offset = packet->GetOffset();
			auto entSize = packet->Read<UInt32>(); // Insurance, in case entity couldn't be created, or data hasn't been received properly
			ent = NET_cl_ent_create_lua(packet, false, true);
			packet->SetOffset(offset + entSize);
		}
		if(ent != nullptr) {
			auto pMapComponent = ent->GetComponent<pragma::MapComponent>();
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

void ClientState::HandleReceiveGameInfo(NetPacket &packet)
{
	if(IsGameActive())
		EndGame();
#ifdef DEBUG_SOCKET
	Con::ccl << "Received Game Information!" << Con::endl;
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
				if(cf->GetType() == ConType::Var) {
					auto *cv = static_cast<ConVar *>(cf);
					if((cv->GetFlags() & ConVarFlags::Replicated) != ConVarFlags::None)
						SetConVar(cvar, value);
				}
			}
			else
				Con::cwar << "Replicated ConVar " << cvar << " doesn't exist" << Con::endl;
		}
	}
	//
	GameModeManager::Initialize();
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

	//if(game == NULL)
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

	auto &sharedNetEventIdToLocal = c_game->GetSharedNetEventIdToLocal();
	sharedNetEventIdToLocal.clear();

	auto numEventIds = packet->Read<uint32_t>();
	sharedNetEventIdToLocal.resize(numEventIds, std::numeric_limits<pragma::NetEventId>::max());
	for(auto i = decltype(numEventIds) {0u}; i < numEventIds; ++i) {
		auto name = packet->ReadString();
		sharedNetEventIdToLocal[i] = c_game->SetupNetEvent(name);
	}

	unsigned int numConCommands = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numConCommands; i++) {
		std::string scmd = packet->ReadString();
		unsigned int id = packet->Read<unsigned int>();
		RegisterServerConVar(scmd, id);
	}

	// Read component manager table
	auto &componentManager = static_cast<pragma::CEntityComponentManager &>(c_game->GetEntityComponentManager());
	auto &componentTypes = componentManager.GetRegisteredComponentTypes();
	auto &svComponentToClComponentTable = componentManager.GetServerComponentIdToClientComponentIdTable();
	auto numTotalSvComponents = packet->Read<uint32_t>();
	auto numComponents = packet->Read<uint32_t>();
	svComponentToClComponentTable.resize(numTotalSvComponents, pragma::CEntityComponentManager::INVALID_COMPONENT);
	for(auto i = decltype(numComponents) {0u}; i < numComponents; ++i) {
		auto name = packet->ReadString();
		auto svId = packet->Read<pragma::ComponentId>();
		auto clComponentId = componentManager.PreRegisterComponentType(name);
		if(clComponentId >= svComponentToClComponentTable.size())
			svComponentToClComponentTable.resize(clComponentId + 1u, pragma::CEntityComponentManager::INVALID_COMPONENT);
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

	BaseEntity *wrld = nwm::read_entity(packet);
	if(wrld != NULL) {
		auto pWorldComponent = wrld->GetComponent<pragma::CWorldComponent>();
		game->SetWorld(pWorldComponent.get());
	}
	ChangeLevel(map.c_str());
	game->ReloadSoundCache();
}

void ClientState::SetGameReady()
{
	if(!m_game)
		return;
	SendPacket("game_ready", pragma::networking::Protocol::SlowReliable);
	m_game->OnGameReady();
}
