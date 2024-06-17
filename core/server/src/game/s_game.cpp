/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/serverstate/serverutil.h"
#include "pragma/game/s_game.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/networking/resourcemanager.h"
#include <fsys/filesystem.h>
#include "luasystem.h"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/entities/player.h"
#include "pragma/cacheinfo.h"
#include "pragma/debug/debugoverlay.h"
#include <pragma/model/brush/brushmesh.h>
#include "pragma/level/mapgeometry.h"
#include "pragma/game/s_game_entities.h"
#include "pragma/encryption/md5.h"
#include <pragma/physics/physobj.h>
#include <pragma/math/surfacematerial.h>
#include "pragma/console/s_convars.h"
#include "pragma/console/s_cvar.h"
#include <pragma/ai/navsystem.h>
#include <pragma/physics/environment.hpp>
#include <pragma/lua/luacallback.h>
#include <pragma/networking/nwm_util.h>
#include <mathutil/umath.h>
#include <pragma/console/convars.h>
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/game/s_game_callback.h"
#include "pragma/lua/classes/s_lua_entity.h"
#include "pragma/ai/ai_behavior.h"
#include "pragma/ai/ai_task_move_to_target.h"
#include "pragma/ai/ai_task_move_random.h"
#include "pragma/ai/ai_task_play_activity.h"
#include "pragma/ai/ai_task_play_sound.h"
#include "pragma/ai/ai_task_debug.h"
#include "pragma/ai/ai_task_wait.h"
#include "pragma/ai/ai_task_random.h"
#include "pragma/ai/ai_task_decorator.h"
#include "pragma/ai/ai_task_turn_to_target.h"
#include "pragma/ai/ai_task_look_at_target.h"
#include "pragma/ai/ai_task_event.hpp"
#include "pragma/lua/s_lua_script_watcher.h"
#include "pragma/model/s_modelmanager.h"
#include "pragma/networking/iserver.hpp"
#include "pragma/networking/iserver_client.hpp"
#include <pragma/lua/luafunction_call.h>
#include "pragma/entities/components/s_entity_component.hpp"
#include "pragma/entities/components/s_vehicle_component.hpp"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/info/s_info_landmark.hpp"
#include "pragma/audio/s_alsound.h"
#include <pragma/networking/enums.hpp>
#include <pragma/networking/error.hpp>
#include <pragma/entities/components/base_gamemode_component.hpp>
#include <pragma/entities/components/global_component.hpp>
#include <pragma/entities/components/base_name_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_character_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <sharedutils/util_file.h>
#include <pragma/engine_version.h>
#include <pragma/util/giblet_create_info.hpp>
#include <servermanager/interface/sv_nwm_manager.hpp>
#include <pragma/entities/components/map_component.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;
extern EntityClassMap<SBaseEntity> *g_ServerEntityFactories;
extern ServerEntityNetworkMap *g_SvEntityNetworkMap;
extern ServerState *server;
extern SGame *s_game;
DLLSERVER pragma::physics::IEnvironment *s_physEnv = nullptr;

SGame::SGame(NetworkState *state) : Game(state)
{
	RegisterCallback<void, SGame *>("OnGameEnd");

	auto &staticCallbacks = get_static_server_callbacks();
	for(auto it = staticCallbacks.begin(); it != staticCallbacks.end(); ++it) {
		auto &name = it->first;
		auto &hCallback = it->second;
		AddCallback(name, hCallback);
	}
	s_physEnv = m_physEnvironment.get();

	m_ents.push_back(NULL); // Slot 0 is reserved
	m_baseEnts.push_back(NULL);

	m_taskManager = std::make_unique<pragma::ai::TaskManager>();
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskMoveToTarget), []() { return std::make_shared<pragma::ai::TaskMoveToTarget>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskPlayAnimation), []() { return std::make_shared<pragma::ai::TaskPlayAnimation>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskPlayActivity), []() { return std::make_shared<pragma::ai::TaskPlayActivity>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskPlayLayeredAnimation), []() { return std::make_shared<pragma::ai::TaskPlayLayeredAnimation>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskPlayLayeredActivity), []() { return std::make_shared<pragma::ai::TaskPlayLayeredActivity>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskMoveRandom), []() { return std::make_shared<pragma::ai::TaskMoveRandom>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskPlaySound), []() { return std::make_shared<pragma::ai::TaskPlaySound>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskDebugPrint), []() { return std::make_shared<pragma::ai::TaskDebugPrint>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskDebugDrawText), []() { return std::make_shared<pragma::ai::TaskDebugDrawText>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskDecorator), []() { return std::make_shared<pragma::ai::TaskDecorator>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskWait), []() { return std::make_shared<pragma::ai::TaskWait>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskTurnToTarget), []() { return std::make_shared<pragma::ai::TaskTurnToTarget>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskRandom), []() { return std::make_shared<pragma::ai::TaskRandom>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskLookAtTarget), []() { return std::make_shared<pragma::ai::TaskLookAtTarget>(); });
	m_taskManager->RegisterTask(typeid(pragma::ai::TaskEvent), []() { return std::make_shared<pragma::ai::TaskEvent>(); }); // These have to correspond with ai::Task enums (See ai_task.h)

	m_cbProfilingHandle = engine->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		auto &cpuProfiler = engine->GetProfiler();
		m_profilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage, CPUProfilingPhase>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler, {pragma::debug::ProfilingStage::Create(cpuProfiler, "Snapshot", &engine->GetProfilingStageManager()->GetProfilerStage(Engine::CPUProfilingPhase::Tick))});
		static_assert(umath::to_integral(CPUProfilingPhase::Count) == 1u, "Added new profiling phase, but did not create associated profiling stage!");
	});
}

SGame::~SGame() {}

void SGame::GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const
{
	std::unordered_map<std::string, SBaseEntity *(*)(void)> *factories = nullptr;
	g_ServerEntityFactories->GetFactories(&factories);
	classes.reserve(classes.size() + factories->size());
	for(auto &pair : *factories)
		classes.push_back(pair.first);
	GetLuaRegisteredEntities(luaClasses);
}

void SGame::OnRemove()
{
	m_flags |= GameFlags::ClosingGame;
	CallCallbacks<void, SGame *>("OnGameEnd", this);
	m_luaCache = nullptr;
	for(unsigned int i = 0; i < m_ents.size(); i++) {
		if(m_ents[i] != NULL) {
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

bool SGame::RunLua(const std::string &lua) { return Game::RunLua(lua, "lua_run"); }

void SGame::OnEntityCreated(BaseEntity *ent) { Game::OnEntityCreated(ent); }

static CVar cvTimescale = GetServerConVar("host_timescale");
float SGame::GetTimeScale() { return cvTimescale->GetFloat(); }

void SGame::SetTimeScale(float t)
{
	Game::SetTimeScale(t);
	NetPacket p;
	p->Write<float>(t);
	server->SendPacket("game_timescale", p, pragma::networking::Protocol::SlowReliable);
}

static void CVAR_CALLBACK_host_timescale(NetworkState *, const ConVar &, float, float val) { s_game->SetTimeScale(val); }
REGISTER_CONVAR_CALLBACK_SV(host_timescale, CVAR_CALLBACK_host_timescale);

void SGame::Initialize()
{
	Game::Initialize();

	InitializeGame();
	SetupLua();
	GenerateLuaCache();
	//NetPacket p;
	//p->Write<float>(GetTimeScale());
	//auto *gameMode = GetGameMode();
	//p->WriteString((gameMode != nullptr) ? gameMode->id : "");
	//server->SendPacket("game_start",p,pragma::networking::Protocol::SlowReliable);
	SetUp();
	if(m_surfaceMaterialManager)
		m_surfaceMaterialManager->Load("scripts/physics/materials.udm");
	m_flags |= GameFlags::GameInitialized;
	CallCallbacks<void, Game *>("OnGameInitialized", this);
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnGameInitialized();
}

void SGame::SetUp()
{
	Game::SetUp();
	auto *entGame = CreateEntity("game");
	assert(entGame != nullptr);
	if(entGame == nullptr) {
		Con::crit << "Unable to create game entity!" << Con::endl;
		// Unreachable
	}
	m_entGame = entGame->GetHandle();
	entGame->Spawn();
}

std::shared_ptr<ModelMesh> SGame::CreateModelMesh() const { return std::make_shared<ModelMesh>(); }
std::shared_ptr<ModelSubMesh> SGame::CreateModelSubMesh() const { return std::make_shared<ModelSubMesh>(); }

bool SGame::LoadMap(const std::string &map, const Vector3 &origin, std::vector<EntityHandle> *entities)
{
	bool b = Game::LoadMap(map, origin, entities);
	if(b == false)
		return false;
	server->SendPacket("map_ready", pragma::networking::Protocol::SlowReliable);
	LoadNavMesh();

	m_flags |= GameFlags::MapLoaded;
	CallCallbacks<void>("OnMapLoaded");
	CallLuaCallbacks<void>("OnMapLoaded");
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnMapInitialized();
	OnMapLoaded();
	return true;
}

static CVar cvSimEnabled = GetServerConVar("sv_physics_simulation_enabled");
bool SGame::IsPhysicsSimulationEnabled() const { return cvSimEnabled->GetBool(); }

std::shared_ptr<pragma::EntityComponentManager> SGame::InitializeEntityComponentManager() { return std::make_shared<pragma::SEntityComponentManager>(); }

pragma::ai::TaskManager &SGame::GetAITaskManager() const { return *m_taskManager; }

void SGame::Think()
{
	Game::Think();
	CallCallbacks<void>("Think");
	CallLuaCallbacks("Think");
	PostThink();
}

void SGame::ChangeLevel(const std::string &mapName, const std::string &landmarkName) { m_changeLevelInfo = {mapName, landmarkName}; }

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage, SGame::CPUProfilingPhase> *SGame::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool SGame::StartProfilingStage(CPUProfilingPhase stage) { return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage); }
bool SGame::StopProfilingStage(CPUProfilingPhase stage) { return m_profilingStageManager && m_profilingStageManager->StopProfilerStage(stage); }

void SGame::Tick()
{
	Game::Tick();

	StartProfilingStage(CPUProfilingPhase::Snapshot);
	SendSnapshot();
	StopProfilingStage(CPUProfilingPhase::Snapshot);

	CallCallbacks<void>("Tick");
	CallLuaCallbacks("Tick");
	PostTick();

	if(m_changeLevelInfo.has_value()) {
		// Write entity state of all entities that have a global name component
		EntityIterator entIt {*this};
		entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::GlobalNameComponent>>();

		std::unordered_map<std::string, udm::PProperty> worldState {};
		for(auto *ent : entIt) {
			auto globalComponent = ent->GetComponent<pragma::GlobalNameComponent>();
			auto &globalName = globalComponent->GetGlobalName();
			auto it = worldState.find(globalName);
			if(it != worldState.end()) {
				Con::cwar << "More than one entity found with global name '" << globalName << "'! This may cause issues." << Con::endl;
				continue;
			}
			auto prop = udm::Property::Create<udm::Element>();
			worldState.insert(std::make_pair(globalName, prop));

			udm::LinkedPropertyWrapper udm {*prop};
			ent->Save(udm);
		}

		auto landmarkName = m_changeLevelInfo->landmarkName;
		auto entItLandmark = EntityIterator {*this};
		entItLandmark.AttachFilter<TEntityIteratorFilterComponent<pragma::SInfoLandmarkComponent>>();
		entItLandmark.AttachFilter<EntityIteratorFilterName>(landmarkName);
		auto it = entItLandmark.begin();
		auto *entLandmark = (it != entItLandmark.end()) ? *it : nullptr;
		auto srcLandmarkPos = (entLandmark != nullptr) ? entLandmark->GetPosition() : Vector3 {};

		auto mapName = m_changeLevelInfo->map;
		server->EndGame();
		server->StartGame(true); // TODO: Keep the current state (i.e. if in multiplayer, stay in multiplayer)

		// Note: 'this' is no longer valid at this point, since the game state has changed
		auto *game = server->GetGameState();
		game->m_flags |= GameFlags::LevelTransition; // Level transition flag has to be set before the map was loaded to make sure it's transmitted to the client(s)
		game->m_preTransitionWorldState = worldState;

		server->ChangeLevel(mapName);

		if(game != nullptr) {
			auto entItLandmark = EntityIterator {*game};
			entItLandmark.AttachFilter<TEntityIteratorFilterComponent<pragma::SInfoLandmarkComponent>>();
			entItLandmark.AttachFilter<EntityIteratorFilterName>(landmarkName);
			auto it = entItLandmark.begin();
			auto *entLandmark = (it != entItLandmark.end()) ? *it : nullptr;
			auto dstLandmarkPos = (entLandmark != nullptr) ? entLandmark->GetPosition() : Vector3 {};
			game->m_deltaTransitionLandmarkOffset = dstLandmarkPos - srcLandmarkPos;

			// Move all global map entities by landmark offset between this level and the previous one
			auto entItGlobalName = EntityIterator {*game};
			// The local player will already be spawned at this point, but doesn't have a map component, so this filter mustn't be included or the player won't be affected
			//entItGlobalName.AttachFilter<TEntityIteratorFilterComponent<pragma::MapComponent>>();
			entItGlobalName.AttachFilter<TEntityIteratorFilterComponent<pragma::GlobalNameComponent>>();
			for(auto *ent : entItGlobalName) {
				auto globalNameComponent = ent->GetComponent<pragma::GlobalNameComponent>();
				ent->SetPosition(ent->GetPosition() + game->m_deltaTransitionLandmarkOffset);
			}
		}
	}
}

bool SGame::IsServer() { return true; }
bool SGame::IsClient() { return false; }

bool SGame::RegisterNetMessage(std::string name)
{
	if(!Game::RegisterNetMessage(name))
		return false;
	NetPacket packet;
	packet->WriteString(name);
	server->SendPacket("luanet_reg", packet, pragma::networking::Protocol::SlowReliable);
	return true;
}

void SGame::InitializeLuaScriptWatcher() { m_scriptWatcher = std::make_unique<SLuaDirectoryWatcherManager>(this); }

void SGame::RegisterGameResource(const std::string &fileName)
{
	//Con::csv<<"RegisterGameResource: "<<fileName<<Con::endl;
	auto fName = FileManager::GetCanonicalizedPath(fileName);
	if(IsValidGameResource(fileName) == true)
		return;
	m_gameResources.push_back(fName);
	auto *sv = server->GetServer();
	if(sv == nullptr)
		return;

	// Send resource to all clients that have already requested it
	for(auto &client : sv->GetClients()) {
		auto &clResources = client->GetScheduledResources();
		auto it = std::find(clResources.begin(), clResources.end(), fileName);
		if(it == clResources.end())
			continue;
		server->SendResourceFile(fileName, {client.get()});
		clResources.erase(it);
	}
}

void SGame::CreateGiblet(const GibletCreateInfo &info)
{
	NetPacket packet {};
	packet->Write<GibletCreateInfo>(info);
	server->SendPacket("create_giblet", packet, pragma::networking::Protocol::FastUnreliable);
}

bool SGame::IsValidGameResource(const std::string &fileName)
{
	auto fName = FileManager::GetCanonicalizedPath(fileName);
	auto it = std::find(m_gameResources.begin(), m_gameResources.end(), fName);
	return (it != m_gameResources.end()) ? true : false;
}

void SGame::UpdateLuaCache(const std::string &fName)
{
	auto *l = GetLuaState();
	auto dstPath = "cache\\" + fName;
	auto includeFlags = fsys::SearchFlags::All;
	auto excludeFlags = static_cast<fsys::SearchFlags>(FSYS_SEARCH_CACHE);
	if(FileManager::Exists(fName.c_str(), includeFlags, excludeFlags))
		FileManager::CopyFile(fName.c_str(), dstPath.c_str()); // Compiled file already exists, just copy it
	else {
		auto luaPath = fName.substr(0, fName.length() - 4) + Lua::SCRIPT_DIRECTORY;
		auto luaPathNoLuaDir = luaPath.substr(4, luaPath.length());
		auto s = LoadLuaFile(luaPathNoLuaDir, includeFlags, excludeFlags);
		if(s == Lua::StatusCode::Ok) {
			FileManager::CreatePath(ufile::get_path_from_filename(dstPath).c_str());
			Lua::compile_file(l, dstPath);
		}
	}
}

void SGame::GenerateLuaCache()
{
	auto &resources = ResourceManager::GetResources();
	filemanager::create_path("cache/" + Lua::SCRIPT_DIRECTORY);
	Con::csv << "Generating lua cache..." << Con::endl;
	for(auto &res : resources) {
		auto &fName = res.fileName;
		std::string ext;
		if(ufile::get_extension(fName, &ext) == true && ext == Lua::FILE_EXTENSION_PRECOMPILED)
			UpdateLuaCache(fName);
	}
}

bool SGame::InitializeGameMode()
{
	if(Game::InitializeGameMode() == false)
		return false;
	auto path = Lua::SCRIPT_DIRECTORY_SLASH + GetGameModeScriptDirectoryPath();

	std::vector<std::string> transferFiles; // Files which need to be transferred to the client

	auto infoFile = path + "\\info.txt";
	if(FileManager::Exists(infoFile))
		transferFiles.push_back(infoFile);

	auto offset = transferFiles.size();
	FileManager::FindFiles((path + "\\*." + Lua::FILE_EXTENSION).c_str(), &transferFiles, nullptr); // Shared Files
	if(Lua::are_precompiled_files_enabled())
		FileManager::FindFiles((path + "\\*." + Lua::FILE_EXTENSION_PRECOMPILED).c_str(), &transferFiles, nullptr);
	for(auto i = offset; i < transferFiles.size(); ++i)
		transferFiles.at(i) = path + '\\' + transferFiles.at(i);

	auto pathClient = path + "\\client";
	offset = transferFiles.size();
	FileManager::FindFiles((pathClient + "\\*." + Lua::FILE_EXTENSION).c_str(), &transferFiles, nullptr); // Clientside Files
	if(Lua::are_precompiled_files_enabled())
		FileManager::FindFiles((pathClient + "\\*." + Lua::FILE_EXTENSION_PRECOMPILED).c_str(), &transferFiles, nullptr);
	for(auto i = offset; i < transferFiles.size(); ++i)
		transferFiles.at(i) = pathClient + '\\' + transferFiles.at(i);

	for(auto &fname : transferFiles)
		ResourceManager::AddResource(fname);
	return true;
}

CacheInfo *SGame::GetLuaCacheInfo() { return m_luaCache.get(); }

void SGame::CreateExplosion(const Vector3 &origin, Float radius, DamageInfo &dmg, const std::function<bool(BaseEntity *, DamageInfo &)> &callback)
{
	SplashDamage(origin, radius, dmg, callback);

	NetPacket p;
	p->Write<Vector3>(origin);
	p->Write<float>(radius);
	server->SendPacket("create_explosion", p, pragma::networking::Protocol::SlowReliable);
}
void SGame::CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force, BaseEntity *attacker, BaseEntity *inflictor, const std::function<bool(BaseEntity *, DamageInfo &)> &callback)
{
	DamageInfo info;
	info.SetForce(Vector3(force, 0.f, 0.f));
	info.SetAttacker(attacker);
	info.SetInflictor(inflictor);
	info.SetDamage(CUInt16(damage));
	info.SetDamageType(DAMAGETYPE::EXPLOSION);
	CreateExplosion(origin, radius, info, callback);
}
void SGame::CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force, const EntityHandle &attacker, const EntityHandle &inflictor, const std::function<bool(BaseEntity *, DamageInfo &)> &callback)
{
	CreateExplosion(origin, radius, damage, force, const_cast<BaseEntity *>(attacker.get()), const_cast<BaseEntity *>(inflictor.get()), callback);
}
void SGame::OnClientDropped(pragma::networking::IServerClient &client, pragma::networking::DropReason reason)
{
	auto *pl = server->GetPlayer(client);
	if(pl == nullptr)
		return;
	auto &ent = pl->GetEntity();
	NetPacket p;
	nwm::write_player(p, pl);
	p->Write<int32_t>(umath::to_integral(reason));
	server->SendPacket("client_dropped", p, pragma::networking::Protocol::SlowReliable, {client, pragma::networking::ClientRecipientFilter::FilterType::Exclude});
	OnPlayerDropped(*pl, reason);
	ent.RemoveSafely();
}

void SGame::ReceiveGameReady(pragma::networking::IServerClient &session, NetPacket &)
{
	auto *pl = GetPlayer(session);
	if(pl == nullptr)
		return;
	pl->SetGameReady(true);
	NetPacket p;
	nwm::write_player(p, pl);
	server->SendPacket("client_ready", p, pragma::networking::Protocol::SlowReliable);
	OnPlayerReady(*pl);
}

void SGame::WriteEntityData(NetPacket &packet, SBaseEntity **ents, uint32_t entCount, pragma::networking::ClientRecipientFilter &rp)
{
	unsigned int numEnts = 0;
	auto posNumEnts = packet->GetSize();
	packet->Write<unsigned int>(numEnts);
	for(auto i = decltype(entCount) {0}; i < entCount; ++i) {
		SBaseEntity *ent = ents[i];
		if(ent != NULL && ent->IsSpawned()) {
			auto pMapComponent = ent->GetComponent<pragma::MapComponent>();
			unsigned int factoryID = g_SvEntityNetworkMap->GetFactoryID(typeid(*ent));
			if(factoryID != 0) {
				packet->Write<Bool>(false);
				packet->Write<unsigned int>(factoryID);
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

void SGame::ReceiveUserInfo(pragma::networking::IServerClient &session, NetPacket &packet)
{
	auto *pl = GetPlayer(session);
	if(pl != NULL)
		return;
	auto plVersion = packet->Read<util::Version>();
	auto version = get_engine_version();
	if(version != plVersion) {
		Con::csv << "Client " << session.GetIdentifier() << " has a different engine version (" << plVersion.ToString() << ") from server's. Dropping client..." << Con::endl;
		server->DropClient(session, pragma::networking::DropReason::Kicked);
		return;
	}

	auto *plEnt = CreateEntity<Player>();
	if(plEnt == nullptr) {
		Con::csv << "Unable to create player entity for client " << session.GetIdentifier() << ". Dropping client..." << Con::endl;
		server->DropClient(session, pragma::networking::DropReason::Kicked);
		return;
	}
	if(plEnt->IsPlayer() == false) {
		Con::csv << "Unable to create player component for client " << session.GetIdentifier() << ". Dropping client..." << Con::endl;
		plEnt->RemoveSafely();
		server->DropClient(session, pragma::networking::DropReason::Kicked);
		return;
	}
	pl = static_cast<pragma::SPlayerComponent *>(plEnt->GetPlayerComponent().get());
	if(session.IsListenServerHost())
		pl->SetLocalPlayer(true);
	session.SetPlayer(*pl);
	pl->SetClientSession(session);
	NetPacket p;
	nwm::write_player(p, pl);
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

	Con::csv << "Player ";
	plEnt->print(Con::cout);
	Con::csv << " authenticated." << Con::endl;
	//unsigned char clPlIdx = pl->GetIndex();
	Con::csv << "Sending Game Information..." << Con::endl;

	pragma::networking::ClientRecipientFilter rp {*pl->GetClientSession()};

	NetPacket packetInf;
	// Write replicated ConVars
	auto &conVars = server->GetConVars();
	uint32_t numReplicated = 0;
	auto offsetNumReplicated = packetInf->GetSize();
	packetInf->Write<uint32_t>(static_cast<uint32_t>(0));
	for(auto &pair : conVars) {
		auto &cf = pair.second;
		if(cf->GetType() == ConType::Var) {
			auto *cv = static_cast<ConVar *>(cf.get());
			if((cv->GetFlags() & ConVarFlags::Replicated) != ConVarFlags::None && cv->GetString() != cv->GetDefault()) {
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

	std::unordered_map<std::string, unsigned int> &conCommandIds = server->GetConCommandIDs();
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
		if(info.IsValid() == false || (info.flags & pragma::ComponentFlags::Networked) == pragma::ComponentFlags::None) {
			--numTypes;
			continue;
		}
		packetInf->WriteString(*info.name);
		packetInf->Write<pragma::ComponentId>(info.id);
	}
	packetInf->Write<uint32_t>(numTypes, &offsetTypes);
	//

	// TODO: Is this obsolete?
	CacheInfo *cache = nullptr; // GetLuaCacheInfo();
	if(cache == NULL || cache->size == 0)
		packetInf->Write<unsigned int>((unsigned int)(0));
	else {
		packetInf->Write<unsigned int>(cache->size);
		packetInf->WriteString(cache->cache);
	}

	WriteEntityData(packetInf, m_ents.data(), m_ents.size(), rp);

	auto *ptrWorld = GetWorld();
	nwm::write_entity(packetInf, (ptrWorld != nullptr) ? &ptrWorld->GetEntity() : nullptr);
	server->SendPacket("gameinfo", packetInf, pragma::networking::Protocol::SlowReliable, rp);
	server->SendPacket("pl_local", p, pragma::networking::Protocol::SlowReliable, session);
	NetPacket tmp {};
	server->SendPacket("game_ready", tmp, pragma::networking::Protocol::SlowReliable, rp);

	NetPacket pJoinedInfo;
	nwm::write_player(pJoinedInfo, pl);
	server->SendPacket("client_joined", pJoinedInfo, pragma::networking::Protocol::SlowReliable);

	if(IsMapInitialized() == true)
		SpawnPlayer(*pl);
	OnPlayerJoined(*pl);

	// Send sound sources
	auto &sounds = server->GetSounds();
	for(auto &sndRef : sounds) {
		auto *snd = dynamic_cast<SALSound *>(&sndRef.get());
		if(snd == nullptr || umath::is_flag_set(snd->GetCreateFlags(), ALCreateFlags::DontTransmit))
			continue;
		server->SendSoundSourceToClient(*snd, true, &rp);
	}
}

pragma::NetEventId SGame::RegisterNetEvent(const std::string &name)
{
	auto id = m_entNetEventManager.RegisterNetEvent(name);
	NetPacket packet;
	packet->WriteString(name);
	packet->Write<pragma::NetEventId>(id);
	server->SendPacket("register_net_event", packet, pragma::networking::Protocol::SlowReliable);
	return id;
}

pragma::NetEventId SGame::FindNetEvent(const std::string &name) const { return m_entNetEventManager.FindNetEvent(name); }
pragma::NetEventId SGame::SetupNetEvent(const std::string &name) { return RegisterNetEvent(name); }
const pragma::NetEventManager &SGame::GetEntityNetEventManager() const { return const_cast<SGame *>(this)->GetEntityNetEventManager(); }
pragma::NetEventManager &SGame::GetEntityNetEventManager() { return m_entNetEventManager; }
const std::vector<std::string> &SGame::GetNetEventIds() const { return const_cast<SGame *>(this)->GetNetEventIds(); }
std::vector<std::string> &SGame::GetNetEventIds() { return m_entNetEventManager.GetNetEventIds(); }

void SGame::SpawnPlayer(pragma::BasePlayerComponent &pl)
{
	auto charComponent = pl.GetEntity().GetComponent<pragma::SCharacterComponent>();
	if(charComponent.expired() == false)
		charComponent.get()->Respawn();
}

void SGame::OnClientConVarChanged(pragma::BasePlayerComponent &pl, std::string cvar, std::string value)
{
	if(cvar == "playername") {
		value = value.substr(0, 20);
		auto nameC = pl.GetEntity().GetNameComponent();
		if(nameC.valid())
			nameC->SetName(value);
		NetPacket p;
		nwm::write_player(p, &pl);
		p->WriteString(value);
		server->SendPacket("pl_changedname", p, pragma::networking::Protocol::SlowReliable);
	}
}

void SGame::DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration) { SDebugRenderer::DrawLine(start, end, color, duration); }
void SGame::DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration)
{
	if(fillColor)
		SDebugRenderer::DrawBox(start, end, ang, *fillColor, colorOutline, duration);
	else
		SDebugRenderer::DrawBox(start, end, ang, colorOutline, duration);
}
void SGame::DrawPlane(const Vector3 &n, float dist, const Color &color, float duration) { SDebugRenderer::DrawPlane(n, dist, color, duration); }
void SGame::DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration) { SDebugRenderer::DrawMesh(meshVerts, color, colorOutline, duration); }

static CVar cvFriction = GetServerConVar("sv_friction");
Float SGame::GetFrictionScale() const { return cvFriction->GetFloat(); }
static CVar cvRestitution = GetServerConVar("sv_restitution");
Float SGame::GetRestitutionScale() const { return cvRestitution->GetFloat(); }
