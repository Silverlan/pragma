// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.server;
import :game;

import :ai;
import :audio;
import :console.register_commands;
import :console.util;
import :core.cache_info;
import :debug;
import :entities;
import :entities.components;
import :entities.registration;
import :model_manager;
import :scripting.lua;
import :server_state;
import pragma.shared;

DLLSERVER pragma::physics::IEnvironment *s_physEnv = nullptr;

#undef CopyFile

static pragma::SGame *g_game = nullptr;
pragma::SGame *pragma::SGame::Get() { return g_game; }
pragma::SGame::SGame(NetworkState *state) : Game(state)
{
	g_game = this;

	RegisterCallback<void, SGame *>("OnGameEnd");
	s_physEnv = m_physEnvironment.get();

	m_ents.push_back(nullptr); // Slot 0 is reserved
	m_baseEnts.push_back(nullptr);

	m_taskManager = std::make_unique<ai::TaskManager>();
	m_taskManager->RegisterTask(typeid(ai::TaskMoveToTarget), []() { return pragma::util::make_shared<ai::TaskMoveToTarget>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskPlayAnimation), []() { return pragma::util::make_shared<ai::TaskPlayAnimation>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskPlayActivity), []() { return pragma::util::make_shared<ai::TaskPlayActivity>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskPlayLayeredAnimation), []() { return pragma::util::make_shared<ai::TaskPlayLayeredAnimation>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskPlayLayeredActivity), []() { return pragma::util::make_shared<ai::TaskPlayLayeredActivity>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskMoveRandom), []() { return pragma::util::make_shared<ai::TaskMoveRandom>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskPlaySound), []() { return pragma::util::make_shared<ai::TaskPlaySound>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskDebugPrint), []() { return pragma::util::make_shared<ai::TaskDebugPrint>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskDebugDrawText), []() { return pragma::util::make_shared<ai::TaskDebugDrawText>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskDecorator), []() { return pragma::util::make_shared<ai::TaskDecorator>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskWait), []() { return pragma::util::make_shared<ai::TaskWait>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskTurnToTarget), []() { return pragma::util::make_shared<ai::TaskTurnToTarget>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskRandom), []() { return pragma::util::make_shared<ai::TaskRandom>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskLookAtTarget), []() { return pragma::util::make_shared<ai::TaskLookAtTarget>(); });
	m_taskManager->RegisterTask(typeid(ai::TaskEvent), []() { return pragma::util::make_shared<ai::TaskEvent>(); }); // These have to correspond with ai::Task enums (See ai_task.h)

	m_cbProfilingHandle = Engine::Get()->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = Engine::Get()->GetProfiler();
		m_profilingStageManager = std::make_unique<debug::ProfilingStageManager<debug::ProfilingStage>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});
}

pragma::SGame::~SGame() { g_game = nullptr; }

void pragma::SGame::GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const
{
	server_entities::ServerEntityRegistry::Instance().GetRegisteredClassNames(classes);
	GetLuaRegisteredEntities(luaClasses);
}

void pragma::SGame::OnRemove()
{
	m_flags |= GameFlags::ClosingGame;
	CallCallbacks<void, SGame *>("OnGameEnd", this);
	m_luaCache = nullptr;
	for(unsigned int i = 0; i < m_ents.size(); i++) {
		if(m_ents[i] != nullptr) {
			m_ents[i]->OnRemove();
			m_ents[i]->Remove();
		}
	}
	if(m_cbProfilingHandle.IsValid())
		m_cbProfilingHandle.Remove();
	s_physEnv = nullptr;
	m_taskManager = nullptr;

	Game::OnRemove();
}

bool pragma::SGame::RunLua(const std::string &lua) { return Game::RunLua(lua, "lua_run"); }

void pragma::SGame::OnEntityCreated(ecs::BaseEntity *ent) { Game::OnEntityCreated(ent); }

static auto cvTimescale = pragma::console::get_server_con_var("host_timescale");
float pragma::SGame::GetTimeScale() { return cvTimescale->GetFloat(); }

void pragma::SGame::SetTimeScale(float t)
{
	Game::SetTimeScale(t);
	NetPacket p;
	p->Write<float>(t);
	ServerState::Get()->SendPacket(networking::net_messages::client::GAME_TIMESCALE, p, networking::Protocol::SlowReliable);
}

static void CVAR_CALLBACK_host_timescale(pragma::NetworkState *, const pragma::console::ConVar &, float, float val) { pragma::SGame::Get()->SetTimeScale(val); }
namespace {
	auto _ = pragma::console::server::register_variable_listener<float>("host_timescale", &CVAR_CALLBACK_host_timescale);
}

void pragma::SGame::Initialize()
{
	Game::Initialize();

	InitializeGame();
	SetupLua();
	GenerateLuaCache();
	//NetPacket p;
	//p->Write<float>(GetTimeScale());
	//auto *gameMode = GetGameMode();
	//p->WriteString((gameMode != nullptr) ? gameMode->id : "");
	//ServerState::Get()->SendPacket(pragma::networking::net_messages::client::GAME_START,p,pragma::networking::Protocol::SlowReliable);
	SetUp();
	if(m_surfaceMaterialManager)
		m_surfaceMaterialManager->Load("scripts/physics/materials.udm");
	m_flags |= GameFlags::GameInitialized;
	CallCallbacks<void, Game *>("OnGameInitialized", this);
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnGameInitialized();
}

void pragma::SGame::SetUp()
{
	Game::SetUp();
	auto *entGame = CreateEntity("game");
	assert(entGame != nullptr);
	if(entGame == nullptr) {
		Con::CRIT << "Unable to create game entity!" << Con::endl;
		// Unreachable
	}
	m_entGame = entGame->GetHandle();
	entGame->Spawn();
}

std::shared_ptr<pragma::geometry::ModelMesh> pragma::SGame::CreateModelMesh() const { return pragma::util::make_shared<geometry::ModelMesh>(); }
std::shared_ptr<pragma::geometry::ModelSubMesh> pragma::SGame::CreateModelSubMesh() const { return pragma::util::make_shared<geometry::ModelSubMesh>(); }

bool pragma::SGame::LoadMap(const std::string &map, const Vector3 &origin, std::vector<EntityHandle> *entities)
{
	bool b = Game::LoadMap(map, origin, entities);
	if(b == false)
		return false;
	ServerState::Get()->SendPacket(networking::net_messages::client::MAP_READY, networking::Protocol::SlowReliable);
	LoadNavMesh();

	m_flags |= GameFlags::MapLoaded;
	CallCallbacks<void>("OnMapLoaded");
	CallLuaCallbacks<void>("OnMapLoaded");
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnMapInitialized();
	OnMapLoaded();
	return true;
}

static auto cvSimEnabled = pragma::console::get_server_con_var("sv_physics_simulation_enabled");
bool pragma::SGame::IsPhysicsSimulationEnabled() const { return cvSimEnabled->GetBool(); }

std::shared_ptr<pragma::EntityComponentManager> pragma::SGame::InitializeEntityComponentManager() { return pragma::util::make_shared<SEntityComponentManager>(); }

pragma::ai::TaskManager &pragma::SGame::GetAITaskManager() const { return *m_taskManager; }

void pragma::SGame::Think()
{
	Game::Think();
	CallCallbacks<void>("Think");
	CallLuaCallbacks("Think");
	PostThink();
}

void pragma::SGame::ChangeLevel(const std::string &mapName, const std::string &landmarkName) { m_changeLevelInfo = {mapName, landmarkName}; }

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *pragma::SGame::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool pragma::SGame::StartProfilingStage(const char *stage) { return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage); }
bool pragma::SGame::StopProfilingStage() { return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(); }

void pragma::SGame::Tick()
{
	Game::Tick();

	StartProfilingStage("Snapshot");
	SendSnapshot();
	StopProfilingStage();

	CallCallbacks<void>("Tick");
	CallLuaCallbacks("Tick");
	PostTick();

	if(m_changeLevelInfo.has_value()) {
		// Write entity state of all entities that have a global name component
		ecs::EntityIterator entIt {*this};
		entIt.AttachFilter<TEntityIteratorFilterComponent<GlobalNameComponent>>();

		std::unordered_map<std::string, udm::PProperty> worldState {};
		for(auto *ent : entIt) {
			auto globalComponent = ent->GetComponent<GlobalNameComponent>();
			auto &globalName = globalComponent->GetGlobalName();
			auto it = worldState.find(globalName);
			if(it != worldState.end()) {
				Con::CWAR << "More than one entity found with global name '" << globalName << "'! This may cause issues." << Con::endl;
				continue;
			}
			auto prop = udm::Property::Create<udm::Element>();
			worldState.insert(std::make_pair(globalName, prop));

			udm::LinkedPropertyWrapper udm {*prop};
			ent->Save(udm);
		}

		auto landmarkName = m_changeLevelInfo->landmarkName;
		auto entItLandmark = ecs::EntityIterator {*this};
		entItLandmark.AttachFilter<TEntityIteratorFilterComponent<SInfoLandmarkComponent>>();
		entItLandmark.AttachFilter<EntityIteratorFilterName>(landmarkName);
		auto it = entItLandmark.begin();
		auto *entLandmark = (it != entItLandmark.end()) ? *it : nullptr;
		auto srcLandmarkPos = (entLandmark != nullptr) ? entLandmark->GetPosition() : Vector3 {};

		auto mapName = m_changeLevelInfo->map;
		ServerState::Get()->EndGame();
		ServerState::Get()->StartGame(true); // TODO: Keep the current state (i.e. if in multiplayer, stay in multiplayer)

		// Note: 'this' is no longer valid at this point, since the game state has changed
		auto *game = ServerState::Get()->GetGameState();
		game->m_flags |= GameFlags::LevelTransition; // Level transition flag has to be set before the map was loaded to make sure it's transmitted to the client(s)
		game->m_preTransitionWorldState = worldState;

		ServerState::Get()->ChangeLevel(mapName);

		if(game != nullptr) {
			auto entItLandmark = ecs::EntityIterator {*game};
			entItLandmark.AttachFilter<TEntityIteratorFilterComponent<SInfoLandmarkComponent>>();
			entItLandmark.AttachFilter<EntityIteratorFilterName>(landmarkName);
			auto it = entItLandmark.begin();
			auto *entLandmark = (it != entItLandmark.end()) ? *it : nullptr;
			auto dstLandmarkPos = (entLandmark != nullptr) ? entLandmark->GetPosition() : Vector3 {};
			game->m_deltaTransitionLandmarkOffset = dstLandmarkPos - srcLandmarkPos;

			// Move all global map entities by landmark offset between this level and the previous one
			auto entItGlobalName = ecs::EntityIterator {*game};
			// The local player will already be spawned at this point, but doesn't have a map component, so this filter mustn't be included or the player won't be affected
			//entItGlobalName.AttachFilter<TEntityIteratorFilterComponent<pragma::MapComponent>>();
			entItGlobalName.AttachFilter<TEntityIteratorFilterComponent<GlobalNameComponent>>();
			for(auto *ent : entItGlobalName) {
				auto globalNameComponent = ent->GetComponent<GlobalNameComponent>();
				ent->SetPosition(ent->GetPosition() + game->m_deltaTransitionLandmarkOffset);
			}
		}
	}
}

bool pragma::SGame::IsServer() { return true; }
bool pragma::SGame::IsClient() { return false; }

bool pragma::SGame::RegisterNetMessage(std::string name)
{
	if(!Game::RegisterNetMessage(name))
		return false;
	NetPacket packet;
	packet->WriteString(name);
	ServerState::Get()->SendPacket(networking::net_messages::client::LUANET_REG, packet, networking::Protocol::SlowReliable);
	return true;
}

void pragma::SGame::InitializeLuaScriptWatcher() { m_scriptWatcher = std::make_unique<SLuaDirectoryWatcherManager>(this); }

void pragma::SGame::RegisterGameResource(const std::string &fileName)
{
	//Con::CSV<<"RegisterGameResource: "<<fileName<<Con::endl;
	auto fName = fs::get_canonicalized_path(fileName);
	if(IsValidGameResource(fileName) == true)
		return;
	m_gameResources.push_back(fName);
	auto *sv = ServerState::Get()->GetServer();
	if(sv == nullptr)
		return;

	// Send resource to all clients that have already requested it
	for(auto &client : sv->GetClients()) {
		auto &clResources = client->GetScheduledResources();
		auto it = std::find(clResources.begin(), clResources.end(), fileName);
		if(it == clResources.end())
			continue;
		ServerState::Get()->SendResourceFile(fileName, {client.get()});
		clResources.erase(it);
	}
}

void pragma::SGame::CreateGiblet(const GibletCreateInfo &info)
{
	NetPacket packet {};
	packet->Write<GibletCreateInfo>(info);
	ServerState::Get()->SendPacket(networking::net_messages::client::CREATE_GIBLET, packet, networking::Protocol::FastUnreliable);
}

bool pragma::SGame::IsValidGameResource(const std::string &fileName)
{
	auto fName = fs::get_canonicalized_path(fileName);
	auto it = std::find(m_gameResources.begin(), m_gameResources.end(), fName);
	return (it != m_gameResources.end()) ? true : false;
}

void pragma::SGame::UpdateLuaCache(const std::string &fName)
{
	auto *l = GetLuaState();
	auto dstPath = "cache\\" + fName;
	auto includeFlags = fs::SearchFlags::All;
	auto excludeFlags = static_cast<fs::SearchFlags>(FSYS_SEARCH_CACHE);
	if(fs::exists(fName, includeFlags, excludeFlags))
		fs::copy_file(fName, dstPath); // Compiled file already exists, just copy it
	else {
		auto luaPath = fName.substr(0, fName.length() - 4) + Lua::SCRIPT_DIRECTORY;
		auto luaPathNoLuaDir = luaPath.substr(4, luaPath.length());
		auto s = LoadLuaFile(luaPathNoLuaDir, includeFlags, excludeFlags);
		if(s == Lua::StatusCode::Ok) {
			fs::create_path(ufile::get_path_from_filename(dstPath));
			Lua::compile_file(l, dstPath);
		}
	}
}

void pragma::SGame::GenerateLuaCache()
{
	auto &resources = networking::ResourceManager::GetResources();
	fs::create_path("cache/" + Lua::SCRIPT_DIRECTORY);
	Con::CSV << "Generating lua cache..." << Con::endl;
	for(auto &res : resources) {
		auto &fName = res.fileName;
		std::string ext;
		if(ufile::get_extension(fName, &ext) == true && ext == Lua::FILE_EXTENSION_PRECOMPILED)
			UpdateLuaCache(fName);
	}
}

bool pragma::SGame::InitializeGameMode()
{
	if(Game::InitializeGameMode() == false)
		return false;
	auto path = Lua::SCRIPT_DIRECTORY_SLASH + GetGameModeScriptDirectoryPath();

	std::vector<std::string> transferFiles; // Files which need to be transferred to the client

	auto infoFile = path + "\\info.txt";
	if(fs::exists(infoFile))
		transferFiles.push_back(infoFile);

	auto offset = transferFiles.size();
	fs::find_files((path + "\\*." + Lua::FILE_EXTENSION), &transferFiles, nullptr); // Shared Files
	if(Lua::are_precompiled_files_enabled())
		fs::find_files((path + "\\*." + Lua::FILE_EXTENSION_PRECOMPILED), &transferFiles, nullptr);
	for(auto i = offset; i < transferFiles.size(); ++i)
		transferFiles.at(i) = path + '\\' + transferFiles.at(i);

	auto pathClient = path + "\\client";
	offset = transferFiles.size();
	fs::find_files((pathClient + "\\*." + Lua::FILE_EXTENSION), &transferFiles, nullptr); // Clientside Files
	if(Lua::are_precompiled_files_enabled())
		fs::find_files((pathClient + "\\*." + Lua::FILE_EXTENSION_PRECOMPILED), &transferFiles, nullptr);
	for(auto i = offset; i < transferFiles.size(); ++i)
		transferFiles.at(i) = pathClient + '\\' + transferFiles.at(i);

	for(auto &fname : transferFiles)
		networking::ResourceManager::AddResource(fname);
	return true;
}

CacheInfo *pragma::SGame::GetLuaCacheInfo() { return m_luaCache.get(); }

void pragma::SGame::CreateExplosion(const Vector3 &origin, Float radius, game::DamageInfo &dmg, const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback)
{
	SplashDamage(origin, radius, dmg, callback);

	NetPacket p;
	p->Write<Vector3>(origin);
	p->Write<float>(radius);
	ServerState::Get()->SendPacket(networking::net_messages::client::CREATE_EXPLOSION, p, networking::Protocol::SlowReliable);
}
void pragma::SGame::CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force, ecs::BaseEntity *attacker, ecs::BaseEntity *inflictor, const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback)
{
	game::DamageInfo info;
	info.SetForce(Vector3(force, 0.f, 0.f));
	info.SetAttacker(attacker);
	info.SetInflictor(inflictor);
	info.SetDamage(CUInt16(damage));
	info.SetDamageType(Explosion);
	CreateExplosion(origin, radius, info, callback);
}
void pragma::SGame::CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force, const EntityHandle &attacker, const EntityHandle &inflictor, const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback)
{
	CreateExplosion(origin, radius, damage, force, const_cast<ecs::BaseEntity *>(attacker.get()), const_cast<ecs::BaseEntity *>(inflictor.get()), callback);
}
void pragma::SGame::OnClientDropped(networking::IServerClient &client, networking::DropReason reason)
{
	auto *pl = ServerState::Get()->GetPlayer(client);
	if(pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	NetPacket p;
	networking::write_player(p, pl);
	p->Write<int32_t>(math::to_integral(reason));
	ServerState::Get()->SendPacket(networking::net_messages::client::CLIENT_DROPPED, p, networking::Protocol::SlowReliable, {client, networking::ClientRecipientFilter::FilterType::Exclude});
	OnPlayerDropped(*pl, reason);
	ent.RemoveSafely();
}

void pragma::SGame::ReceiveGameReady(networking::IServerClient &session, NetPacket &)
{
	auto *pl = GetPlayer(session);
	if(pl == nullptr)
		return;
	pl->SetGameReady(true);
	NetPacket p;
	networking::write_player(p, pl);
	ServerState::Get()->SendPacket(networking::net_messages::client::CLIENT_READY, p, networking::Protocol::SlowReliable);
	OnPlayerReady(*pl);
}

void pragma::SGame::WriteEntityData(NetPacket &packet, SBaseEntity **ents, uint32_t entCount, networking::ClientRecipientFilter &rp)
{
	unsigned int numEnts = 0;
	auto posNumEnts = packet->GetSize();
	packet->Write<unsigned int>(numEnts);
	for(auto i = decltype(entCount) {0}; i < entCount; ++i) {
		SBaseEntity *ent = ents[i];
		if(ent != nullptr && ent->IsSpawned()) {
			auto pMapComponent = ent->GetComponent<MapComponent>();
			auto factoryID = server_entities::ServerEntityRegistry::Instance().GetNetworkFactoryID(typeid(*ent));
			if(factoryID != std::nullopt) {
				packet->Write<Bool>(false);
				packet->Write<unsigned int>(*factoryID);
				packet->Write<unsigned int>(ent->GetIndex());
				packet->Write<unsigned int>(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
				ent->SendData(packet, rp);
				numEnts++;
			}
			else if(ent->IsScripted() && ent->IsShared()) {
				packet->Write<Bool>(true);
				auto offset = packet->GetSize();
				packet->Write<UInt32>(UInt32(0));
				packet->WriteString(*ent->GetClass());
				packet->Write<unsigned int>(ent->GetIndex());
				packet->Write<unsigned int>(pMapComponent.valid() ? pMapComponent->GetMapIndex() : 0u);
				ent->SendData(packet, rp);
				auto dataSize = packet->GetSize() - offset;
				packet->Write<UInt32>(dataSize, &offset);
				numEnts++;
			}
		}
	}
	packet->Write<unsigned int>(numEnts, &posNumEnts);
}

void pragma::SGame::ReceiveUserInfo(networking::IServerClient &session, NetPacket &packet)
{
	auto *pl = GetPlayer(session);
	if(pl != nullptr)
		return;
	auto plVersion = packet->Read<util::Version>();
	auto version = get_engine_version();
	if(version != plVersion) {
		Con::CSV << "Client " << session.GetIdentifier() << " has a different engine version (" << plVersion.ToString() << ") from server's. Dropping client..." << Con::endl;
		ServerState::Get()->DropClient(session, networking::DropReason::Kicked);
		return;
	}

	auto *plEnt = CreateEntity<Player>();
	if(plEnt == nullptr) {
		Con::CSV << "Unable to create player entity for client " << session.GetIdentifier() << ". Dropping client..." << Con::endl;
		ServerState::Get()->DropClient(session, networking::DropReason::Kicked);
		return;
	}
	if(plEnt->IsPlayer() == false) {
		Con::CSV << "Unable to create player component for client " << session.GetIdentifier() << ". Dropping client..." << Con::endl;
		plEnt->RemoveSafely();
		ServerState::Get()->DropClient(session, networking::DropReason::Kicked);
		return;
	}
	pl = static_cast<SPlayerComponent *>(plEnt->GetPlayerComponent().get());
	if(session.IsListenServerHost())
		pl->SetLocalPlayer(true);
	session.SetPlayer(*pl);
	pl->SetClientSession(session);
	NetPacket p;
	networking::write_player(p, pl);
	if(packet->Read<unsigned char>() == 1) // Does the player have UDP available?
	{
		unsigned short portUDP = packet->Read<unsigned short>();
		pl->SetUDPPort(portUDP);
	}
	std::string name = packet->ReadString();
	name = name.substr(0, 20);
	if(name.empty())
		name = "player";
	auto nameC = plEnt->GetNameComponent();
	if(nameC.valid())
		nameC->SetName(name);
	plEnt->Spawn();
	pl->SetAuthed(true);
	std::unordered_map<std::string, std::string> *cvars;
	pl->GetConVars(&cvars);
	unsigned int numUserInfo = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numUserInfo; i++) {
		std::string cmd = packet->ReadString();
		std::string val = packet->ReadString();
		(*cvars)[cmd] = val;
		OnClientConVarChanged(*pl, cmd, val);
	}

	Con::CSV << "Player " << *plEnt << " authenticated." << Con::endl;
	//unsigned char clPlIdx = pl->GetIndex();
	Con::CSV << "Sending Game Information..." << Con::endl;

	networking::ClientRecipientFilter rp {*pl->GetClientSession()};

	NetPacket packetInf;
	// Write replicated ConVars
	auto &conVars = ServerState::Get()->GetConVars();
	uint32_t numReplicated = 0;
	auto offsetNumReplicated = packetInf->GetSize();
	packetInf->Write<uint32_t>(static_cast<uint32_t>(0));
	for(auto &pair : conVars) {
		auto &cf = pair.second;
		if(cf->GetType() == console::ConType::Var) {
			auto *cv = static_cast<console::ConVar *>(cf.get());
			if((cv->GetFlags() & console::ConVarFlags::Replicated) != console::ConVarFlags::None && cv->GetString() != cv->GetDefault()) {
				auto id = cv->GetID();
				packetInf->Write<uint32_t>(id);
				if(id == 0)
					packetInf->WriteString(pair.first);
				packetInf->WriteString(cv->GetString());
				++numReplicated;
			}
		}
	}
	packetInf->Write<uint32_t>(numReplicated, &offsetNumReplicated);
	//

	packetInf->WriteString(GetMapName());
	packetInf->Write<GameFlags>(GetGameFlags());
	packetInf->Write<double>(CurTime());

	unsigned int numMessages = CUInt32(m_luaNetMessageIndex.size());
	packetInf->Write<unsigned int>(numMessages - 1);
	for(unsigned int i = 1; i < numMessages; i++)
		packetInf->WriteString(m_luaNetMessageIndex[i]);

	auto &netEventIds = GetNetEventIds();
	packetInf->Write<uint32_t>(netEventIds.size());
	for(auto &name : netEventIds)
		packetInf->WriteString(name);

	std::unordered_map<std::string, unsigned int> &conCommandIds = ServerState::Get()->GetConCommandIDs();
	packetInf->Write<unsigned int>(CUInt32(conCommandIds.size()));
	std::unordered_map<std::string, unsigned int>::iterator it;
	for(it = conCommandIds.begin(); it != conCommandIds.end(); it++) {
		packetInf->WriteString(it->first);
		packetInf->Write<unsigned int>(it->second);
	}

	// Write component manager table
	auto &componentManager = GetEntityComponentManager();
	auto &componentTypes = componentManager.GetRegisteredComponentTypes();
	auto numTypes = componentTypes.size();
	packetInf->Write<uint32_t>(componentTypes.size());
	auto offsetTypes = packetInf->GetOffset();
	packetInf->Write<uint32_t>(numTypes);
	for(auto &info : componentTypes) {
		if(info->IsValid() == false || (info->flags & ComponentFlags::Networked) == ComponentFlags::None) {
			--numTypes;
			continue;
		}
		packetInf->WriteString(*info->name);
		packetInf->Write<ComponentId>(info->id);
	}
	packetInf->Write<uint32_t>(numTypes, &offsetTypes);
	//

	// TODO: Is this obsolete?
	CacheInfo *cache = nullptr; // GetLuaCacheInfo();
	if(cache == nullptr || cache->size == 0)
		packetInf->Write<unsigned int>((unsigned int)(0));
	else {
		packetInf->Write<unsigned int>(cache->size);
		packetInf->WriteString(cache->cache);
	}

	WriteEntityData(packetInf, m_ents.data(), m_ents.size(), rp);

	auto *ptrWorld = GetWorld();
	networking::write_entity(packetInf, (ptrWorld != nullptr) ? &ptrWorld->GetEntity() : nullptr);
	ServerState::Get()->SendPacket(networking::net_messages::client::GAMEINFO, packetInf, networking::Protocol::SlowReliable, rp);
	ServerState::Get()->SendPacket(networking::net_messages::client::PL_LOCAL, p, networking::Protocol::SlowReliable, session);
	NetPacket tmp {};
	ServerState::Get()->SendPacket(networking::net_messages::client::GAME_READY, tmp, networking::Protocol::SlowReliable, rp);

	NetPacket pJoinedInfo;
	networking::write_player(pJoinedInfo, pl);
	ServerState::Get()->SendPacket(networking::net_messages::client::CLIENT_JOINED, pJoinedInfo, networking::Protocol::SlowReliable);

	if(IsMapInitialized() == true)
		SpawnPlayer(*pl);
	OnPlayerJoined(*pl);

	// Send sound sources
	auto &sounds = ServerState::Get()->GetSounds();
	for(auto &sndRef : sounds) {
		auto *snd = dynamic_cast<audio::SALSound *>(&sndRef.get());
		if(snd == nullptr || math::is_flag_set(snd->GetCreateFlags(), audio::ALCreateFlags::DontTransmit))
			continue;
		ServerState::Get()->SendSoundSourceToClient(*snd, true, &rp);
	}
}

pragma::NetEventId pragma::SGame::RegisterNetEvent(const std::string &name)
{
	auto id = m_entNetEventManager.RegisterNetEvent(name);
	NetPacket packet;
	packet->WriteString(name);
	packet->Write<NetEventId>(id);
	ServerState::Get()->SendPacket(networking::net_messages::client::REGISTER_NET_EVENT, packet, networking::Protocol::SlowReliable);
	return id;
}

pragma::NetEventId pragma::SGame::FindNetEvent(const std::string &name) const { return m_entNetEventManager.FindNetEvent(name); }
pragma::NetEventId pragma::SGame::SetupNetEvent(const std::string &name) { return RegisterNetEvent(name); }
const pragma::NetEventManager &pragma::SGame::GetEntityNetEventManager() const { return const_cast<SGame *>(this)->GetEntityNetEventManager(); }
pragma::NetEventManager &pragma::SGame::GetEntityNetEventManager() { return m_entNetEventManager; }
const std::vector<std::string> &pragma::SGame::GetNetEventIds() const { return const_cast<SGame *>(this)->GetNetEventIds(); }
std::vector<std::string> &pragma::SGame::GetNetEventIds() { return m_entNetEventManager.GetNetEventIds(); }

void pragma::SGame::SpawnPlayer(BasePlayerComponent &pl)
{
	auto charComponent = pl.GetEntity().GetComponent<SCharacterComponent>();
	if(charComponent.expired() == false)
		charComponent.get()->Respawn();
}

void pragma::SGame::OnClientConVarChanged(BasePlayerComponent &pl, std::string cvar, std::string value)
{
	if(cvar == "playername") {
		value = value.substr(0, 20);
		auto nameC = pl.GetEntity().GetNameComponent();
		if(nameC.valid())
			nameC->SetName(value);
		NetPacket p;
		networking::write_player(p, &pl);
		p->WriteString(value);
		ServerState::Get()->SendPacket(networking::net_messages::client::PL_CHANGEDNAME, p, networking::Protocol::SlowReliable);
	}
}

void pragma::SGame::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration) { debug::SDebugRenderer::DrawLine(start, end, color, duration); }
void pragma::SGame::DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration)
{
	if(fillColor)
		debug::SDebugRenderer::DrawBox(start, end, ang, *fillColor, colorOutline, duration);
	else
		debug::SDebugRenderer::DrawBox(start, end, ang, colorOutline, duration);
}
void pragma::SGame::DrawPlane(const Vector3 &n, float dist, const Color &color, float duration) { debug::SDebugRenderer::DrawPlane(n, dist, color, duration); }
void pragma::SGame::DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration) { debug::SDebugRenderer::DrawMesh(meshVerts, color, colorOutline, duration); }

static auto cvFriction = pragma::console::get_server_con_var("sv_friction");
Float pragma::SGame::GetFrictionScale() const { return cvFriction->GetFloat(); }
static auto cvRestitution = pragma::console::get_server_con_var("sv_restitution");
Float pragma::SGame::GetRestitutionScale() const { return cvRestitution->GetFloat(); }

void pragma::SGame::HandleLuaNetPacket(networking::IServerClient &session, NetPacket &packet)
{
	unsigned int ID = packet->Read<unsigned int>();
	if(ID == 0)
		return;
	auto *pl = GetPlayer(session);
	if(pl == nullptr)
		return;
	std::string *ident = GetNetMessageIdentifier(ID);
	if(ident == nullptr)
		return;
	std::unordered_map<std::string, int>::iterator i = m_luaNetMessages.find(*ident);
	if(i == m_luaNetMessages.end()) {
		Con::CWAR << Con::PREFIX_SERVER << "Unhandled lua net message: " << *ident << Con::endl;
		return;
	}
	ProtectedLuaCall(
	  [&i, &pl, &packet](lua::State *l) {
		  lua::raw_get(l, Lua::RegistryIndex, i->second);
		  luabind::object(l, packet).push(l);
		  pl->PushLuaObject(l);
		  return Lua::StatusCode::Ok;
	  },
	  0);
}
