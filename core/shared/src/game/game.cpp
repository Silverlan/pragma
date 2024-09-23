/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/networkstate/networkstate.h"
#include <pragma/console/convars.h>
#include "luasystem.h"
#include "pragma/physics/physobj.h"
#include "pragma/entities/baseentity.h"
#include "pragma/model/brush/brushmesh.h"
#include "pragma/level/mapgeometry.h"
#include <pragma/engine.h>
#include "pragma/ai/navsystem.h"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/contact.hpp"
#include "pragma/physics/constraint.hpp"
#include "pragma/lua/libraries/ltimer.h"
#include "pragma/game/gamemode/gamemodemanager.h"
#include "pragma/logging.hpp"
#include <pragma/console/convars.h>
#include "pragma/console/engine_cvar.h"
#include "pragma/game/game_callback.h"
#include "pragma/game/animation_update_manager.hpp"
#include "pragma/lua/luafunction_call.h"
#include "pragma/addonsystem/addonsystem.h"
#include "pragma/lua/lua_script_watcher.h"
#include "pragma/util/resource_watcher.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/input/inkeys.h"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_gamemode_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/base_static_bvh_cache_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/prop/prop_base.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/baseworld.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/level/level_info.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/debug/debug_performance_profiler.hpp"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/class_manager.hpp"
#include "pragma/util/util_bsp_tree.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/asset_types/world.hpp"
#include "pragma/model/model.h"
#include "pragma/model/modelmanager.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/asset/util_asset.hpp"
#include "pragma/util/util_game.hpp"
#include "pragma/debug/intel_vtune.hpp"
#include <material_manager2.hpp>
#include <sharedutils/util_library.hpp>
#include <fsys/ifile.hpp>
#include <luainterface.hpp>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

std::optional<std::string> Lua::VarToString(lua_State *lua, int n)
{
	auto t = GetType(lua, n);
	switch(t) {
	case Lua::Type::None:
		return "none";
	case Lua::Type::Nil:
		return "nil";
	case Lua::Type::Bool:
		return "bool (" + std::string {ToBool(lua, n) ? "true" : "false"} + ")";
	case Lua::Type::LightUserData:
		return "lightuserdata";
	case Lua::Type::Number:
		return "number (" + std::to_string(ToNumber(lua, n)) + ")";
	case Lua::Type::String:
		return "string (" + std::string {ToString(lua, n)} + ")";
	case Lua::Type::Table:
		return "table";
	case Lua::Type::Function:
		return "function";
	case Lua::Type::UserData:
		return "userdata";
	case Lua::Type::Thread:
		return "thread";
	default:
		return "other (" + std::string {GetTypeName(lua, n)} + ")";
	}
	return {};
}
void Lua::VarDump(lua_State *lua, int n)
{
	auto str = VarToString(lua, n);
	if(str.has_value())
		Con::cout << *str;
}

std::optional<std::string> Lua::StackToString(lua_State *lua)
{
	std::string str;
	int top = GetStackTop(lua);
	str += "------------ LUA STACKDUMP ------------\n";
	str += "Values in stack: " + std::to_string(top) + "\n";
	for(int i = 1; i <= top; i++) {
		str += "\t" + std::to_string(i) + ": ";
		auto var = VarToString(lua, i);
		if(var.has_value())
			str += *var;
		str += "\n";
	}
	str += "---------------------------------------\n";
	return str;
}

void Lua::StackDump(lua_State *lua)
{
	auto str = StackToString(lua);
	if(str.has_value())
		Con::cout << *str << Con::endl;
}

std::optional<std::string> Lua::TableToString(lua_State *lua, int n)
{
	if(n < 0)
		n = Lua::GetStackTop(lua) + n + 1;
	std::string str;
	str += "------------ LUA TABLEDUMP ------------\n";
	if(n <= 0) {
		str += "INVALID STACK INDEX (" + std::to_string(n) + ")\n";
		return str;
	}
	if(!Lua::IsTable(lua, n)) {
		str += "VALUE " + std::to_string(n) + " ON STACK IS A ";
		auto var = VarToString(lua, n);
		if(var.has_value())
			str += *var;
		str += ", NOT A TABLE!\n";
		return str;
	}
	Lua::PushNil(lua);
	while(Lua::GetNextPair(lua, n) != 0) {
		str += "\t";
		auto var = VarToString(lua, -2);
		if(var.has_value())
			str += *var;
		str += " = ";
		var = VarToString(lua, -1);
		if(var.has_value())
			str += *var;
		str += "\n";
		Lua::Pop(lua, 1); // We need the key at the top for the next iteration
	}
	str += "---------------------------------------\n";
	return str;
}

void Lua::TableDump(lua_State *lua, int n)
{
	if(n < 0)
		n = Lua::GetStackTop(lua) + n + 1;
	Con::cout << "------------ LUA TABLEDUMP ------------" << Con::endl;
	if(n <= 0) {
		Con::cout << "INVALID STACK INDEX (" << n << ")" << Con::endl;
		return;
	}
	if(!Lua::IsTable(lua, n)) {
		Con::cout << "VALUE " << n << " ON STACK IS A ";
		VarDump(lua, n);
		Con::cout << ", NOT A TABLE!" << Con::endl;
		return;
	}
	Lua::PushNil(lua);
	while(Lua::GetNextPair(lua, n) != 0) {
		Con::cout << "\t";
		VarDump(lua, -2);
		Con::cout << " = ";
		VarDump(lua, -1);
		Con::cout << Con::endl;
		Lua::Pop(lua, 1); // We need the key at the top for the next iteration
	}
	Con::cout << "---------------------------------------" << Con::endl;
}

////////////////

extern DLLNETWORK Engine *engine;
Game::Game(NetworkState *state)
{
	m_stateNetwork = state;
	m_mapInfo.name = "";
	m_mapInfo.md5 = "";
	m_luaNetMessageIndex.push_back("invalid");
	m_luaEnts = std::make_unique<LuaEntityManager>();
	m_ammoTypes = std::make_unique<AmmoTypeManager>();

	RegisterCallback<void>("Tick");
	RegisterCallback<void>("Think");

	RegisterCallback<void, lua_State *>("OnLuaReleased");
	RegisterCallback<void, pragma::BasePlayerComponent *>("OnPlayerReady");
	RegisterCallback<void, pragma::BasePlayerComponent *, pragma::networking::DropReason>("OnPlayerDropped");
	RegisterCallback<void, pragma::BasePlayerComponent *>("OnPlayerJoined");
	RegisterCallback<void, BaseEntity *>("OnEntityCreated");
	RegisterCallback<void>("PrePhysicsSimulate");
	RegisterCallback<void>("PostPhysicsSimulate");

	RegisterCallback<void, BaseEntity *, uint16_t, uint16_t>("OnEntityHealthChanged");
	RegisterCallback<void, BaseEntity *, std::reference_wrapper<DamageInfo>>("OnEntityTakeDamage");
	RegisterCallback<void, BaseEntity *, std::reference_wrapper<DamageInfo>, uint16_t, uint16_t>("OnEntityTakenDamage");
	RegisterCallback<void, pragma::BaseAIComponent *, DamageInfo *>("OnNPCDeath");

	RegisterCallback<void, pragma::BasePlayerComponent *, DamageInfo *>("OnPlayerDeath");
	RegisterCallback<void, pragma::BasePlayerComponent *>("OnPlayerSpawned");

	RegisterCallbackWithOptionalReturn<bool, pragma::ActionInputControllerComponent *, Action, bool>("OnActionInput");

	RegisterCallback<void, lua_State *>("OnLuaInitialized");
	RegisterCallback<void, BaseEntity *>("OnEntitySpawned");
	RegisterCallback<void, Game *>("OnGameInitialized");
	RegisterCallback<void>("OnMapLoaded");
	RegisterCallback<void>("OnPreLoadMap");
	RegisterCallback<void>("OnGameReady");
	RegisterCallback<void, ALSound *>("OnSoundCreated");
	RegisterCallback<void, std::reference_wrapper<std::shared_ptr<Model>>>("OnModelLoaded");

	RegisterCallback<void>("EndGame");

	LoadSoundScripts("game_sounds_generic.udm");
	LoadSoundScripts("fx_physics_impact.udm");
}

Game::~Game() {}

void Game::OnRemove()
{
	pragma::BaseAIComponent::ReleaseNavThread();
	CallCallbacks<void>("OnLuaReleased", GetLuaState());
	m_luaCallbacks.clear();
	m_luaEnts = nullptr;
	m_componentManager = nullptr;
	ClearTimers(); // Timers have to be removed before the lua state is closed
	m_cvarCallbacks.clear();

	// We have to clear all of the collision mesh shapes, because they may
	// contain luabind objects, which have to be destroyed before we destroy the
	// lua state.
	// TODO: Implement this properly!
	auto &mdlManager = GetNetworkState()->GetModelManager();
	for(auto &pair : mdlManager.GetCache()) {
		auto asset = mdlManager.GetAsset(pair.second);
		if(!asset)
			continue;
		for(auto &colMesh : pragma::asset::ModelManager::GetAssetObject(*asset)->GetCollisionMeshes())
			colMesh->ClearShape();
	}

	auto *state = m_lua->GetState();
	auto identifier = m_lua->GetIdentifier();
	GetNetworkState()->ClearConsoleCommandOverrides();
	GetNetworkState()->TerminateLuaModules(state);
	pragma::BaseLuaBaseEntityComponent::ClearMembers(state);
	m_surfaceMaterialManager = nullptr; // Has to be destroyed before physics environment!
	m_physEnvironment = nullptr;        // Physics environment has to be destroyed before the Lua state! (To make sure Lua-handles are destroyed)
	m_luaClassManager = nullptr;
	m_lua = nullptr;
	GetNetworkState()->DeregisterLuaModules(state, identifier); // Has to be called AFTER Lua instance has been released!
	if(m_cbProfilingHandle.IsValid())
		m_cbProfilingHandle.Remove();
}

void Game::GetLuaRegisteredEntities(std::vector<std::string> &luaClasses) const
{
	auto &manager = GetLuaEntityManager();
	auto &ents = manager.GetRegisteredEntities();
	luaClasses.reserve(luaClasses.size() + ents.size());
	for(auto &pair : ents)
		luaClasses.push_back(pair.first);
}

void Game::OnMapLoaded() {}

void Game::InitializeLuaScriptWatcher() { m_scriptWatcher = std::make_unique<LuaDirectoryWatcherManager>(this); }

BaseEntity *Game::GetGameModeEntity() { return m_entGamemode.get(); }
BaseEntity *Game::GetGameEntity() { return m_entGame.get(); }
bool Game::IsGameInitialized() const { return (m_flags & GameFlags::GameInitialized) != GameFlags::None; }
bool Game::IsMapLoaded() const { return (m_flags & GameFlags::MapLoaded) != GameFlags::None; }

void Game::OnPlayerReady(pragma::BasePlayerComponent &pl)
{
	CallCallbacks<void, pragma::BasePlayerComponent *>("OnPlayerReady", &pl);
	CallLuaCallbacks<void, luabind::object>("OnPlayerReady", pl.GetLuaObject());
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnPlayerReady(pl);
}

void Game::OnPlayerDropped(pragma::BasePlayerComponent &pl, pragma::networking::DropReason reason)
{
	CallCallbacks<void, pragma::BasePlayerComponent *, pragma::networking::DropReason>("OnPlayerDropped", &pl, reason);
	CallLuaCallbacks<void, luabind::object, std::underlying_type_t<decltype(reason)>>("OnPlayerDropped", pl.GetLuaObject(), umath::to_integral(reason));
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnPlayerDropped(pl, reason);
}

void Game::OnPlayerJoined(pragma::BasePlayerComponent &pl)
{
	CallCallbacks<void, pragma::BasePlayerComponent *>("OnPlayerJoined", &pl);
	CallLuaCallbacks<void, luabind::object>("OnPlayerJoined", pl.GetLuaObject());
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnPlayerJoined(pl);
}

unsigned char Game::GetPlayerCount() { return m_numPlayers; }

LuaEntityManager &Game::GetLuaEntityManager() { return *m_luaEnts.get(); }

pragma::AnimationUpdateManager &Game::GetAnimationUpdateManager() { return *m_animUpdateManager; }

const GameModeInfo *Game::GetGameMode() const { return const_cast<Game *>(this)->GetGameMode(); }
GameModeInfo *Game::GetGameMode() { return m_gameMode; }
void Game::SetGameMode(const std::string &gameMode)
{
	auto *info = GameModeManager::GetGameModeInfo(gameMode);
	m_gameMode = info;

	if(info) {
		for(auto &pair : info->gameMountPriorities)
			util::set_mounted_game_priority(pair.first, pair.second);
	}
}

void Game::SetupEntity(BaseEntity *) {}

bool Game::IsMultiPlayer() const { return engine->IsMultiPlayer(); }
bool Game::IsSinglePlayer() const { return engine->IsSinglePlayer(); }

const pragma::physics::IEnvironment *Game::GetPhysicsEnvironment() const { return const_cast<Game *>(this)->GetPhysicsEnvironment(); }
pragma::physics::IEnvironment *Game::GetPhysicsEnvironment() { return m_physEnvironment.get(); }

void Game::OnEntityCreated(BaseEntity *ent)
{
	CallCallbacks<void, BaseEntity *>("OnEntityCreated", ent);
	auto &o = ent->GetLuaObject();
	CallLuaCallbacks<void, luabind::object>("OnEntityCreated", o);
}

Vector3 &Game::GetGravity() { return m_gravity; }
void Game::SetGravity(Vector3 &gravity) { m_gravity = gravity; }

std::vector<std::string> *Game::GetLuaNetMessageIndices() { return &m_luaNetMessageIndex; }

LuaDirectoryWatcherManager &Game::GetLuaScriptWatcher() { return *m_scriptWatcher; }
ResourceWatcherManager &Game::GetResourceWatcher() { return GetNetworkState()->GetResourceWatcher(); }

const std::shared_ptr<pragma::nav::Mesh> &Game::GetNavMesh() const { return const_cast<Game *>(this)->GetNavMesh(); }
std::shared_ptr<pragma::nav::Mesh> &Game::GetNavMesh() { return m_navMesh; }

std::shared_ptr<pragma::nav::Mesh> Game::LoadNavMesh(const std::string &fname) { return pragma::nav::Mesh::Load(*this, fname); }

bool Game::LoadNavMesh(bool bReload)
{
	if(m_navMesh != nullptr) {
		if(bReload == false)
			return true;
		m_navMesh = nullptr;
	}
	std::string path = "maps\\";
	path += GetMapName();

	auto pathAscii = path + "." + std::string {pragma::nav::PNAV_EXTENSION_ASCII};
	auto pathBinary = path + "." + std::string {pragma::nav::PNAV_EXTENSION_BINARY};
	path = FileManager::Exists(pathBinary) ? pathBinary : pathAscii;
	Con::cout << "Loading navigation mesh..." << Con::endl;

	m_navMesh = LoadNavMesh(path);
	if(m_navMesh == nullptr)
		Con::cwar << "Unable to load navigation mesh!" << Con::endl;
	pragma::BaseAIComponent::ReloadNavThread(*this);
	return m_navMesh != nullptr;
}

void Game::InitializeMountedAddon(const AddonInfo &addonInfo) const { m_scriptWatcher->MountDirectory(addonInfo.GetAbsolutePath() + "/lua", true); }

void Game::Initialize()
{
	m_componentManager = InitializeEntityComponentManager();
	InitializeEntityComponents(*m_componentManager);

	m_animUpdateManager = std::make_unique<pragma::AnimationUpdateManager>(*this);

	InitializeLuaScriptWatcher();
	m_scriptWatcher->MountDirectory("lua");
	auto &addons = AddonSystem::GetMountedAddons();
	for(auto &info : addons)
		InitializeMountedAddon(info);

	LoadSoundScripts("fx.udm");
}

void Game::OnInitialized()
{
	auto &mdlManager = GetNetworkState()->GetModelManager();
	mdlManager.PreloadAsset("error");
}

void Game::SetUp() {}

static bool check_validity(pragma::BasePhysicsComponent &physC) { return !umath::is_flag_set(physC.BaseEntityComponent::GetStateFlags(), pragma::BaseEntityComponent::StateFlags::Removed); }
static bool check_validity(BaseEntity *ent)
{
	if(!ent || ent->IsRemoved())
		return false;
	auto physC = ent->GetPhysicsComponent();
	return (physC != nullptr && check_validity(*physC));
}
class PhysEventCallback : public pragma::physics::IEventCallback {
  public:
	// Called if contact report is enabled for a collision object and it
	// collided with another actor. This is NOT called for triggers!
	virtual void OnContact(const pragma::physics::ContactInfo &contactInfo) override
	{
		if(contactInfo.collisionObj0.IsExpired() || contactInfo.collisionObj1.IsExpired())
			return;
		auto *physObj0 = contactInfo.collisionObj0->GetPhysObj();
		auto *entC0 = physObj0 ? physObj0->GetOwner() : nullptr;
		auto *touchC0 = entC0 ? static_cast<pragma::BaseTouchComponent *>(entC0->GetEntity().FindComponent("touch").get()) : nullptr;
		if(touchC0 == nullptr || check_validity(&entC0->GetEntity()) == false)
			return;
		touchC0->Contact(contactInfo);
	}

	// Called whenever a constraint is broken
	virtual void OnConstraintBroken(pragma::physics::IConstraint &constraint) override
	{
		auto *ent = constraint.GetEntity();
		if(check_validity(ent) == false)
			return;
		// TODO: Check constraint component
	}

	// Called when an actor has started touching another for the first time.
	virtual void OnStartTouch(pragma::physics::ICollisionObject &o0, pragma::physics::ICollisionObject &o1) override
	{
		// TODO: TocuhComponent:SetTouchEnabled -> Rely to physicxs
		// TODO: Make sure EndTouch is called when actor is removed and still touching!
		auto *physObj0 = o0.GetPhysObj();
		auto *entC0 = physObj0 ? physObj0->GetOwner() : nullptr;
		auto *touchC0 = entC0 ? static_cast<pragma::BaseTouchComponent *>(entC0->GetEntity().FindComponent("touch").get()) : nullptr;

		auto *physObj1 = o1.GetPhysObj();
		auto *entC1 = physObj1 ? physObj1->GetOwner() : nullptr;
		if(touchC0 == nullptr || entC1 == nullptr || !check_validity(&entC0->GetEntity()) || !check_validity(&entC1->GetEntity()))
			return;
		touchC0->StartTouch(entC1->GetEntity(), *physObj1, o0, o1);
	}

	// Called when an actor has stopped touching another actor.
	virtual void OnEndTouch(pragma::physics::ICollisionObject &o0, pragma::physics::ICollisionObject &o1) override
	{
		auto *physObj0 = o0.GetPhysObj();
		auto *entC0 = physObj0 ? physObj0->GetOwner() : nullptr;
		auto *touchC0 = entC0 ? static_cast<pragma::BaseTouchComponent *>(entC0->GetEntity().FindComponent("touch").get()) : nullptr;

		auto *physObj1 = o1.GetPhysObj();
		auto *entC1 = physObj1 ? physObj1->GetOwner() : nullptr;
		if(touchC0 == nullptr || entC1 == nullptr || !check_validity(&entC0->GetEntity()) || !check_validity(&entC1->GetEntity()))
			return;
		touchC0->EndTouch(entC1->GetEntity(), *physObj1, o0, o1);
	}

	virtual void OnWake(pragma::physics::ICollisionObject &o) override
	{
		auto *physObj = o.GetPhysObj();
		auto *ent = physObj ? physObj->GetOwner() : nullptr;
		auto *physC = ent ? ent->GetEntity().GetPhysicsComponent() : nullptr;
		if(physC == nullptr || !check_validity(*physC))
			return;
		physC->OnWake();
	}
	virtual void OnSleep(pragma::physics::ICollisionObject &o) override
	{
		auto *physObj = o.GetPhysObj();
		auto *ent = physObj ? physObj->GetOwner() : nullptr;
		auto *physC = ent ? ent->GetEntity().GetPhysicsComponent() : nullptr;
		if(physC == nullptr || !check_validity(*physC))
			return;
		physC->OnSleep();
	}
};

void Game::InitializeGame()
{
	InitializeLua(); // Lua has to be initialized completely before any entites are created

	auto physEngineName = GetConVarString("phys_engine");
	auto physEngineLibName = pragma::physics::IEnvironment::GetPhysicsEngineModuleLocation(physEngineName);
	spdlog::info("Loading physics module '{}'...", physEngineLibName);
	std::string err;
	auto dllHandle = GetNetworkState()->InitializeLibrary(physEngineLibName, &err);
	if(dllHandle) {
		auto *fInitPhysicsEngine = dllHandle->FindSymbolAddress<void (*)(NetworkState &, std::unique_ptr<pragma::physics::IEnvironment, void (*)(pragma::physics::IEnvironment *)> &)>("initialize_physics_engine");
		if(fInitPhysicsEngine != nullptr)
			fInitPhysicsEngine(*GetNetworkState(), m_physEnvironment);
		else
			Con::cerr << "Unable to initialize physics engine '" << physEngineName << "': Function 'initialize_physics_engine' not found!" << Con::endl;
	}
	else
		Con::cerr << "Unable to initialize physics engine '" << physEngineName << "': " << err << Con::endl;
	if(m_physEnvironment) {
		m_surfaceMaterialManager = std::make_unique<SurfaceMaterialManager>(*m_physEnvironment);
		m_physEnvironment->SetEventCallback(std::make_unique<PhysEventCallback>());

		auto &tireTypeManager = m_physEnvironment->GetTireTypeManager();
		auto &surfTypeManager = m_physEnvironment->GetSurfaceTypeManager();

		std::string err;
		auto udmData = util::load_udm_asset("scripts/physics/tire_types.udm", &err);
		if(udmData) {
			auto &data = *udmData;
			auto udm = data.GetAssetData().GetData();
			for(auto pair : udm.ElIt()) {
				auto &identifier = pair.key;
				auto hTireType = tireTypeManager.RegisterType(std::string {identifier});
				if(hTireType.IsExpired())
					continue;
				auto udmFrictionModifiers = pair.property["friction_modifiers"];
				for(auto pair : udmFrictionModifiers.ElIt()) {
					auto &surfaceType = pair.key;
					auto surfType = surfTypeManager.RegisterType(std::string {surfaceType});
					auto friction = 1.f;
					pair.property(friction);
					hTireType->SetFrictionModifier(*surfType, friction);
				}
			}
		}
	}

	m_cbProfilingHandle = engine->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		std::string postFix = IsClient() ? " (CL)" : " (SV)";
		auto &cpuProfiler = engine->GetProfiler();
		m_profilingStageManager = std::make_unique<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});
}

void Game::GetEntities(std::vector<BaseEntity *> **ents) { *ents = &m_baseEnts; }
void Game::GetSpawnedEntities(std::vector<BaseEntity *> *ents)
{
	std::vector<BaseEntity *> *baseEnts;
	GetEntities(&baseEnts);
	for(unsigned int i = 0; i < baseEnts->size(); i++) {
		BaseEntity *ent = (*baseEnts)[i];
		if(ent != NULL && ent->IsSpawned())
			ents->push_back(ent);
	}
}

NetworkState *Game::GetNetworkState() { return m_stateNetwork; }

void Game::UpdateTime()
{
	float timeScale = GetTimeScale();
	m_ctCur.Update(timeScale);
	m_ctReal.Update();
	m_tCur = CDouble(m_ctCur());
	m_tReal = CDouble(m_ctReal());
	m_tDelta = CDouble(m_tCur - m_tLast);
	m_tDeltaReal = CDouble(m_tReal - m_tLastReal);
}
void Game::Think()
{
	UpdateTime();

	m_scriptWatcher->Poll(); // TODO: Don't do this every frame?
}
void Game::PostThink()
{
	m_tLast = m_tCur;
	m_tLastReal = m_tReal;
}

double &Game::GetLastThink() { return m_tLast; }
double &Game::GetLastTick() { return m_tLastTick; }

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *Game::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool Game::StartProfilingStage(const char *stage)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("stage_" + std::string {stage});
#endif
	return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage);
}
bool Game::StopProfilingStage()
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().EndTask();
#endif
	return m_profilingStageManager && m_profilingStageManager->StopProfilerStage();
}

std::string Game::GetMapName() { return m_mapInfo.name; }

const std::vector<BaseEntity *> &Game::GetBaseEntities() const { return const_cast<Game *>(this)->GetBaseEntities(); }
std::vector<BaseEntity *> &Game::GetBaseEntities() { return m_baseEnts; }
std::size_t Game::GetBaseEntityCount() const { return m_numEnts; }

void Game::ScheduleEntityForRemoval(BaseEntity &ent) { m_entsScheduledForRemoval.push(ent.GetHandle()); }

void Game::UpdateAnimations(double dt) { m_animUpdateManager->UpdateAnimations(dt); }

void Game::Tick()
{
	StartProfilingStage("Tick");
	if((m_flags & GameFlags::InitialTick) != GameFlags::None) {
		m_flags &= ~GameFlags::InitialTick;
		m_tDeltaTick = 0.0f; // First tick is essentially 'skipped' to avoid physics errors after the world has been loaded
	}
	else
		m_tDeltaTick = (1.f / engine->GetTickRate()) * GetTimeScale(); //m_tCur -m_tLastTick;
	for(auto *ent : m_baseEnts) {
		if(ent != nullptr && ent->IsSpawned())
			ent->ResetStateChangeFlags();
	}

	// Order:
	// Animations are updated before logic and physics, because:
	// - They may affect logic/physics-based properties like entity positions or rotations
	// - They may generate logic-based animation events
	// TODO: This is current inefficient because we're not really doing anything
	// while the animations are being calculated, resulting in wasted CPU cycles.
	StartProfilingStage("Animations");
	UpdateAnimations(m_tDeltaTick);
	StopProfilingStage(); // Animations

	StartProfilingStage("Physics");

	auto &awakePhysics = GetAwakePhysicsComponents();
	for(auto &hPhysC : awakePhysics) {
		if(hPhysC.expired() || hPhysC->GetPhysicsType() == PHYSICSTYPE::NONE)
			continue;
		hPhysC->PrePhysicsSimulate(); // Has to be called BEFORE PhysicsUpdate (This is where stuff like Character movement is handled)!
	}

	for(auto &hPhysC : awakePhysics) {
		if(hPhysC.expired() || hPhysC->GetPhysicsType() == PHYSICSTYPE::NONE)
			continue;
		hPhysC->PhysicsUpdate(m_tDeltaTick); // Has to be called AFTER PrePhysicsSimulate (This is where physics objects are updated)!
	}

	CallCallbacks("PrePhysicsSimulate");
	CallLuaCallbacks("PrePhysicsSimulate");
	StartProfilingStage("PhysicsSimulation");
	if(IsPhysicsSimulationEnabled() == true && m_physEnvironment) {
		static int maxSteps = 1;
		m_tPhysDeltaRemainder = m_physEnvironment->StepSimulation(CFloat(m_tDeltaTick + m_tPhysDeltaRemainder), maxSteps, CFloat(m_tDeltaTick));
	}
	StopProfilingStage(); // PhysicsSimulation
	CallCallbacks("PostPhysicsSimulate");
	CallLuaCallbacks("PostPhysicsSimulate");

	for(auto it = awakePhysics.begin(); it != awakePhysics.end();) {
		auto &hPhysC = *it;
		if(hPhysC.expired() || hPhysC->GetPhysicsType() == PHYSICSTYPE::NONE) {
			++it;
			continue;
		}
		auto keepAwake = hPhysC->PostPhysicsSimulate();
		hPhysC->UpdatePhysicsData(); // Has to be before Think (Requires updated physics).
		if(keepAwake == false)
			it = awakePhysics.erase(it);
		else
			++it;
	}

	StopProfilingStage(); // Physics

	while(m_entsScheduledForRemoval.empty() == false) {
		auto &hEnt = m_entsScheduledForRemoval.front();
		if(hEnt.valid())
			hEnt->Remove();

		m_entsScheduledForRemoval.pop();
	}

	StartProfilingStage("GameObjectLogic");

	// Perform some cleanup
	pragma::BaseEntityComponentSystem::Cleanup();

	auto &logicComponents = GetEntityTickComponents();
	// Note: During the loop, new items may be appended to the end of logicComponents, but no elements
	// may be erased from outside sources. If an element is removed, it's set to nullptr.
	auto shouldProfile = (m_profilingStageManager != nullptr);
	for(auto i = decltype(logicComponents.size()) {0u}; i < logicComponents.size();) {
		auto *c = logicComponents[i];
		if(c == nullptr) {
			logicComponents.erase(logicComponents.begin() + i);
			continue;
		}
		if(m_tCur < c->GetNextTick()) {
			++i;
			continue;
		}
		if(shouldProfile) {
			auto *cInfo = c->GetComponentInfo();
			StartProfilingStage(cInfo->name.str);
		}
		auto res = c->Tick(m_tDeltaTick);
		if(shouldProfile)
			StopProfilingStage();
		if(res == false) {
			logicComponents.erase(logicComponents.begin() + i);
			continue;
		}
		++i;
	}

	StopProfilingStage(); // GameObjectLogic

	StartProfilingStage("Timers");
	UpdateTimers();
	StopProfilingStage(); // Timers
	//if(GetNetworkState()->IsClient())
	//	return;
	StopProfilingStage(); // Tick
}
void Game::PostTick() { m_tLastTick = m_tCur; }

void Game::SetGameFlags(GameFlags flags) { m_flags = flags; }
Game::GameFlags Game::GetGameFlags() const { return m_flags; }

bool Game::IsMapInitialized() const { return (m_flags & GameFlags::MapInitialized) != GameFlags::None; }
bool Game::IsGameReady() const { return (m_flags & GameFlags::GameReady) != GameFlags::None; }

const MapInfo &Game::GetMapInfo() const { return m_mapInfo; }

void Game::UpdatePackagePaths()
{
	auto path = util::Path::CreatePath(util::get_program_path());
	std::vector<std::string> packagePaths = {};
	auto &addons = AddonSystem::GetMountedAddons();
	packagePaths.reserve(2 + addons.size());
	packagePaths.push_back((path + "lua/?.lua").GetString());
	packagePaths.push_back((path + "lua/modules/?.lua").GetString());

	for(auto &addonInfo : addons) {
		auto path = util::Path::CreatePath(addonInfo.GetAbsolutePath()) + "lua/modules/?.lua";
		packagePaths.push_back(path.GetString());
	}
	auto package = luabind::object {luabind::globals(GetLuaState())["package"]};
	if(Lua::GetType(package) == Lua::Type::Nil)
		return;
	package["path"] = ustring::implode(packagePaths, ";");

#ifdef _WIN32
	std::string ext = ".dll";
#else
	std::string ext = ".so";
#endif
	package["cpath"] = (path + ("modules/?" + ext)).GetString();
}

bool Game::LoadSoundScripts(const char *file) { return m_stateNetwork->LoadSoundScripts(file, true); }
bool Game::LoadMap(const std::string &map, const Vector3 &origin, std::vector<EntityHandle> *entities)
{
	auto normPath = pragma::asset::get_normalized_path(map, pragma::asset::Type::Map);
	std::string format;
	auto filePath = pragma::asset::find_file(normPath, pragma::asset::Type::Map, &format);
	if(filePath.has_value() == false) {
		static auto bPort = true;
		if(bPort == true) {
			Con::cwar << "Map '" << map << "' not found." << Con::endl;
			auto path = pragma::asset::relative_path_to_absolute_path(normPath, pragma::asset::Type::Map);
			if(util::port_source2_map(GetNetworkState(), path.GetString()) == false && util::port_hl2_map(GetNetworkState(), path.GetString()) == false)
				Con::cwar << " Loading empty map..." << Con::endl;
			else {
				Con::cwar << Con::endl;
				Con::cout << "Successfully ported HL2 map " << path.GetString() << "!" << Con::endl;
				bPort = false;
				auto r = LoadMap(map);
				bPort = true;
				return r;
			}
		}
		m_flags |= GameFlags::MapInitialized;
		return false;
	}
	m_mapInfo.name = map;
	m_mapInfo.fileName = pragma::asset::relative_path_to_absolute_path(*filePath, pragma::asset::Type::Map).GetString();
	util::ScopeGuard sg {[this]() { m_flags |= GameFlags::MapInitialized; }};

	auto error = [this, &map](const std::string_view &msg) { Con::cwar << "Unable to load map '" << map << "': " << msg << Con::endl; };

	auto f = FileManager::OpenFile(m_mapInfo.fileName.c_str(), "rb");
	if(f == nullptr) {
		error("Unable to open file '" + m_mapInfo.fileName + "'!");
		return false;
	}

	auto worldData = pragma::asset::WorldData::Create(*GetNetworkState());
	if(pragma::asset::matches_format(format, pragma::asset::FORMAT_MAP_BINARY) || pragma::asset::matches_format(format, pragma::asset::FORMAT_MAP_ASCII)) {
		auto udmData = util::load_udm_asset(std::make_unique<fsys::File>(f));
		std::string err;
		if(udmData == nullptr || worldData->LoadFromAssetData(udmData->GetAssetData(), pragma::asset::EntityData::Flags::None, err) == false) {
			error(err);
			return false;
		}
	}
	else if(pragma::asset::matches_format(format, pragma::asset::FORMAT_MAP_LEGACY)) {
		std::string err;
		auto success = worldData->Read(f, pragma::asset::EntityData::Flags::None, &err);
		if(success == false) {
			error(err);
			return false;
		}
	}
	else {
		error("Unknown error");
		return false;
	}

	// Load sound scripts
	std::string soundScript = "soundscapes_";
	soundScript += map;
	soundScript += ".txt";
	LoadSoundScripts(soundScript.c_str());

	// Load entities
	Con::cout << "Loading entities..." << Con::endl;

	pragma::BaseStaticBvhCacheComponent *bvhC = nullptr;
	if(IsClient()) {
		auto *entBvh = CreateEntity("entity");
		assert(entBvh);
		bvhC = static_cast<pragma::BaseStaticBvhCacheComponent *>(entBvh->AddComponent("static_bvh_cache").get());
		if(!bvhC) {
			entBvh->Remove();
			entBvh = nullptr;
		}
		else
			entBvh->Spawn();
	}

	std::vector<EntityHandle> ents {};
	InitializeMapEntities(*worldData, ents);
	for(auto &hEnt : ents) {
		if(hEnt.valid() == false)
			continue;
		if(bvhC && hEnt->IsStatic())
			bvhC->AddEntity(*hEnt);
		hEnt->Spawn();
	}
	for(auto &hEnt : ents) {
		if(hEnt.valid() == false || hEnt->IsSpawned() == false)
			continue;
		hEnt->OnSpawn();
	}
	InitializeWorldData(*worldData);
	return true;
}

void Game::InitializeWorldData(pragma::asset::WorldData &worldData) {}
void Game::InitializeMapEntities(pragma::asset::WorldData &worldData, std::vector<EntityHandle> &outEnt)
{
	auto &entityData = worldData.GetEntities();
	outEnt.reserve(entityData.size());
	for(auto &entData : entityData) {
		auto *ent = CreateMapEntity(*entData);
		if(ent == nullptr)
			continue;
		outEnt.push_back(ent->GetHandle());
	}
}

std::shared_ptr<Model> Game::CreateModel(const std::string &mdl) const { return m_stateNetwork->GetModelManager().CreateModel(mdl); }
std::shared_ptr<Model> Game::CreateModel(bool bAddReference) const { return m_stateNetwork->GetModelManager().CreateModel("", bAddReference); }
bool Game::PrecacheModel(const std::string &mdl)
{
	spdlog::info("Precaching model '{}'...", mdl);
	auto *asset = GetNetworkState()->GetModelManager().FindCachedAsset(mdl);
	if(asset)
		return true;
	auto loadInfo = std::make_unique<pragma::asset::ModelLoadInfo>();
	loadInfo->onLoaded = [this](util::Asset &asset) {
		auto mdl = pragma::asset::ModelManager::GetAssetObject(asset);
		CallCallbacks<void, std::reference_wrapper<std::shared_ptr<Model>>>("OnModelLoaded", mdl);
		CallLuaCallbacks<void, std::shared_ptr<Model>>("OnModelLoaded", mdl);
	};
	auto r = GetNetworkState()->GetModelManager().PreloadAsset(mdl, std::move(loadInfo));
	return r;
}
std::shared_ptr<Model> Game::LoadModel(const std::string &mdl, bool bReload)
{
	if(mdl.empty())
		return nullptr;
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	debug::get_domain().BeginTask("load_model");
	util::ScopeGuard sgVtune {[]() { debug::get_domain().EndTask(); }};
#endif
	spdlog::debug("Loading model '{}'...", mdl);
	auto *asset = GetNetworkState()->GetModelManager().FindCachedAsset(mdl);
	if(asset)
		return pragma::asset::ModelManager::GetAssetObject(*asset);
	auto &mdlMananger = GetNetworkState()->GetModelManager();
	util::FileAssetManager::PreloadResult result;
	auto r = bReload ? mdlMananger.ReloadAsset(mdl, nullptr, &result) : mdlMananger.LoadAsset(mdl, nullptr, &result);
	if(r != nullptr) {
		CallCallbacks<void, std::reference_wrapper<std::shared_ptr<Model>>>("OnModelLoaded", r);
		CallLuaCallbacks<void, std::shared_ptr<Model>>("OnModelLoaded", r);
	}
	else {
		Con::cwar << "Failed to load model '" << mdl << "': " << magic_enum::enum_name(result.result) << Con::endl;
		if(result.errorMessage)
			Con::cwar << "Error Message:\n" << *result.errorMessage << Con::endl;
	}
	return r;
}

void Game::OnGameReady()
{
	// All assets have been loaded, now clear the unused assets (if there was a previous map)
	Con::cout << "Clearing unused assets..." << Con::endl;
	auto numModelsCleared = GetNetworkState()->GetModelManager().ClearUnused();
	auto numMaterialsCleared = GetNetworkState()->GetMaterialManager().ClearUnused();
	Con::cout << numModelsCleared << " models and " << numMaterialsCleared << " materials have been cleared from cache!" << Con::endl;

	m_ctCur.Reset();
	m_ctReal.Reset();
	CallCallbacks<void>("OnGameReady");

	m_flags |= GameFlags::GameReady;
	CallLuaCallbacks("OnGameReady");
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnGameReady();
}

void Game::SetWorld(pragma::BaseWorldComponent *entWorld)
{
	if(!entWorld)
		return;
	m_worldComponents.push_back(entWorld->GetHandle<pragma::BaseWorldComponent>());
}

std::vector<pragma::ComponentHandle<pragma::BasePhysicsComponent>> &Game::GetAwakePhysicsComponents() { return m_awakePhysicsEntities; }

const pragma::EntityComponentManager &Game::GetEntityComponentManager() const { return const_cast<Game *>(this)->GetEntityComponentManager(); }
pragma::EntityComponentManager &Game::GetEntityComponentManager() { return *m_componentManager; }

SurfaceMaterial &Game::CreateSurfaceMaterial(const std::string &identifier, Float friction, Float restitution) { return m_surfaceMaterialManager->Create(identifier, friction, restitution); }
SurfaceMaterial *Game::GetSurfaceMaterial(const std::string &id) { return m_surfaceMaterialManager ? m_surfaceMaterialManager->GetMaterial(id) : nullptr; }
SurfaceMaterial *Game::GetSurfaceMaterial(UInt32 id)
{
	if(m_surfaceMaterialManager == nullptr)
		return nullptr;
	auto &materials = m_surfaceMaterialManager->GetMaterials();
	if(id >= materials.size())
		return nullptr;
	return &materials[id];
}
std::vector<SurfaceMaterial> *Game::GetSurfaceMaterials() { return m_surfaceMaterialManager ? &m_surfaceMaterialManager->GetMaterials() : nullptr; }

double &Game::RealTime() { return m_tReal; }
double &Game::CurTime() { return m_tCur; }
double &Game::ServerTime() { return CurTime(); }
double &Game::DeltaTime() { return m_tDelta; }
double &Game::DeltaRealTime() { return m_stateNetwork->DeltaTime(); }
double &Game::LastThink() { return m_tLast; }
double &Game::LastTick() { return m_tLastTick; }
double &Game::DeltaTickTime() { return m_tDeltaTick; }

float Game::GetTimeScale() { return 1.f; }
void Game::SetTimeScale(float t) { m_stateNetwork->SetConVar("host_timescale", std::to_string(t)); }

ConConf *Game::GetConVar(const std::string &scmd) { return m_stateNetwork->GetConVar(scmd); }
int Game::GetConVarInt(const std::string &scmd) { return m_stateNetwork->GetConVarInt(scmd); }
std::string Game::GetConVarString(const std::string &scmd) { return m_stateNetwork->GetConVarString(scmd); }
float Game::GetConVarFloat(const std::string &scmd) { return m_stateNetwork->GetConVarFloat(scmd); }
bool Game::GetConVarBool(const std::string &scmd) { return m_stateNetwork->GetConVarBool(scmd); }
ConVarFlags Game::GetConVarFlags(const std::string &scmd) { return m_stateNetwork->GetConVarFlags(scmd); }
