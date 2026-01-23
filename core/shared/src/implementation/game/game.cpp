// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :game.game;

std::optional<std::string> Lua::VarToString(lua::State *lua, int n)
{
	auto t = GetType(lua, n);
	switch(t) {
	case Type::None:
		return "none";
	case Type::Nil:
		return "nil";
	case Type::Bool:
		return "bool (" + std::string {ToBool(lua, n) ? "true" : "false"} + ")";
	case Type::LightUserData:
		return "lightuserdata";
	case Type::Number:
		return "number (" + std::to_string(ToNumber(lua, n)) + ")";
	case Type::String:
		return "string (" + std::string {ToString(lua, n)} + ")";
	case Type::Table:
		return "table";
	case Type::Function:
		return "function";
	case Type::UserData:
		return "userdata";
	case Type::Thread:
		return "thread";
	default:
		return "other (" + std::string {GetTypeName(lua, n)} + ")";
	}
	return {};
}
void Lua::VarDump(lua::State *lua, int n)
{
	auto str = VarToString(lua, n);
	if(str.has_value())
		Con::COUT << *str;
}

std::optional<std::string> Lua::StackToString(lua::State *lua)
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

void Lua::StackDump(lua::State *lua)
{
	auto str = StackToString(lua);
	if(str.has_value())
		Con::COUT << *str << Con::endl;
}

std::optional<std::string> Lua::TableToString(lua::State *lua, int n)
{
	if(n < 0)
		n = GetStackTop(lua) + n + 1;
	std::string str;
	str += "------------ LUA TABLEDUMP ------------\n";
	if(n <= 0) {
		str += "INVALID STACK INDEX (" + std::to_string(n) + ")\n";
		return str;
	}
	if(!IsTable(lua, n)) {
		str += "VALUE " + std::to_string(n) + " ON STACK IS A ";
		auto var = VarToString(lua, n);
		if(var.has_value())
			str += *var;
		str += ", NOT A TABLE!\n";
		return str;
	}
	PushNil(lua);
	while(GetNextPair(lua, n) != 0) {
		str += "\t";
		auto var = VarToString(lua, -2);
		if(var.has_value())
			str += *var;
		str += " = ";
		var = VarToString(lua, -1);
		if(var.has_value())
			str += *var;
		str += "\n";
		Pop(lua, 1); // We need the key at the top for the next iteration
	}
	str += "---------------------------------------\n";
	return str;
}

void Lua::TableDump(lua::State *lua, int n)
{
	if(n < 0)
		n = GetStackTop(lua) + n + 1;
	Con::COUT << "------------ LUA TABLEDUMP ------------" << Con::endl;
	if(n <= 0) {
		Con::COUT << "INVALID STACK INDEX (" << n << ")" << Con::endl;
		return;
	}
	if(!IsTable(lua, n)) {
		Con::COUT << "VALUE " << n << " ON STACK IS A ";
		VarDump(lua, n);
		Con::COUT << ", NOT A TABLE!" << Con::endl;
		return;
	}
	PushNil(lua);
	while(GetNextPair(lua, n) != 0) {
		Con::COUT << "\t";
		VarDump(lua, -2);
		Con::COUT << " = ";
		VarDump(lua, -1);
		Con::COUT << Con::endl;
		Pop(lua, 1); // We need the key at the top for the next iteration
	}
	Con::COUT << "---------------------------------------" << Con::endl;
}

////////////////

pragma::Game::Game(NetworkState *state)
{
	m_stateNetwork = state;
	m_mapInfo.name = "";
	m_mapInfo.md5 = "";
	m_luaNetMessageIndex.push_back("invalid");
	m_luaEnts = std::make_unique<LuaEntityManager>();
	m_ammoTypes = std::make_unique<game::AmmoTypeManager>();

	RegisterCallback<void>("Tick");
	RegisterCallback<void>("Think");

	RegisterCallback<void, lua::State *>("OnLuaReleased");
	RegisterCallback<void, BasePlayerComponent *>("OnPlayerReady");
	RegisterCallback<void, BasePlayerComponent *, networking::DropReason>("OnPlayerDropped");
	RegisterCallback<void, BasePlayerComponent *>("OnPlayerJoined");
	RegisterCallback<void, ecs::BaseEntity *>("OnEntityCreated");
	RegisterCallback<void>("PrePhysicsSimulate");
	RegisterCallback<void>("PostPhysicsSimulate");

	RegisterCallback<void, ecs::BaseEntity *, uint16_t, uint16_t>("OnEntityHealthChanged");
	RegisterCallback<void, ecs::BaseEntity *, std::reference_wrapper<game::DamageInfo>>("OnEntityTakeDamage");
	RegisterCallback<void, ecs::BaseEntity *, std::reference_wrapper<game::DamageInfo>, uint16_t, uint16_t>("OnEntityTakenDamage");
	RegisterCallback<void, BaseAIComponent *, game::DamageInfo *>("OnNPCDeath");

	RegisterCallback<void, BasePlayerComponent *, game::DamageInfo *>("OnPlayerDeath");
	RegisterCallback<void, BasePlayerComponent *>("OnPlayerSpawned");

	RegisterCallbackWithOptionalReturn<bool, ActionInputControllerComponent *, Action, bool>("OnActionInput");

	RegisterCallback<void, lua::State *>("OnLuaInitialized");
	RegisterCallback<void, ecs::BaseEntity *>("OnEntitySpawned");
	RegisterCallback<void, Game *>("OnGameInitialized");
	RegisterCallback<void>("OnMapLoaded");
	RegisterCallback<void>("OnPreLoadMap");
	RegisterCallback<void>("OnGameReady");
	RegisterCallback<void, audio::ALSound *>("OnSoundCreated");
	RegisterCallback<void, std::reference_wrapper<std::shared_ptr<asset::Model>>>("OnModelLoaded");

	RegisterCallback<void>("EndGame");

	LoadSoundScripts("game_sounds_generic.udm");
	LoadSoundScripts("fx_physics_impact.udm");
}

pragma::Game::~Game() {}

void pragma::Game::OnRemove()
{
	BaseAIComponent::ReleaseNavThread();
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
		for(auto &colMesh : asset::ModelManager::GetAssetObject(*asset)->GetCollisionMeshes())
			colMesh->ClearShape();
	}

	auto *state = m_lua->GetState();
	auto identifier = m_lua->GetIdentifier();
	GetNetworkState()->ClearConsoleCommandOverrides();
	GetNetworkState()->TerminateLuaModules(state);
	BaseLuaBaseEntityComponent::ClearMembers(state);
	m_surfaceMaterialManager = nullptr; // Has to be destroyed before physics environment!
	m_physEnvironment = nullptr;        // Physics environment has to be destroyed before the Lua state! (To make sure Lua-handles are destroyed)
	m_luaClassManager = nullptr;
	m_lua = nullptr;
	GetNetworkState()->DeregisterLuaModules(state, identifier); // Has to be called AFTER Lua instance has been released!
	if(m_cbProfilingHandle.IsValid())
		m_cbProfilingHandle.Remove();
}

bool pragma::Game::IsServer() { return false; }
bool pragma::Game::IsClient() { return false; }

void pragma::Game::GetLuaRegisteredEntities(std::vector<std::string> &luaClasses) const
{
	auto &manager = GetLuaEntityManager();
	auto &ents = manager.GetRegisteredEntities();
	luaClasses.reserve(luaClasses.size() + ents.size());
	for(auto &pair : ents)
		luaClasses.push_back(pair.first);
}

void pragma::Game::OnMapLoaded() {}

void pragma::Game::InitializeLuaScriptWatcher() { m_scriptWatcher = std::make_unique<LuaDirectoryWatcherManager>(this); }

pragma::ecs::BaseEntity *pragma::Game::GetGameModeEntity() { return m_entGamemode.get(); }
pragma::ecs::BaseEntity *pragma::Game::GetGameEntity() { return m_entGame.get(); }
bool pragma::Game::IsGameInitialized() const { return (m_flags & GameFlags::GameInitialized) != GameFlags::None; }
bool pragma::Game::IsMapLoaded() const { return (m_flags & GameFlags::MapLoaded) != GameFlags::None; }

void pragma::Game::OnPlayerReady(BasePlayerComponent &pl)
{
	CallCallbacks<void, BasePlayerComponent *>("OnPlayerReady", &pl);
	CallLuaCallbacks<void, luabind::object>("OnPlayerReady", pl.GetLuaObject());
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnPlayerReady(pl);
}

void pragma::Game::OnPlayerDropped(BasePlayerComponent &pl, networking::DropReason reason)
{
	CallCallbacks<void, BasePlayerComponent *, networking::DropReason>("OnPlayerDropped", &pl, reason);
	CallLuaCallbacks<void, luabind::object, std::underlying_type_t<decltype(reason)>>("OnPlayerDropped", pl.GetLuaObject(), math::to_integral(reason));
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnPlayerDropped(pl, reason);
}

void pragma::Game::OnPlayerJoined(BasePlayerComponent &pl)
{
	CallCallbacks<void, BasePlayerComponent *>("OnPlayerJoined", &pl);
	CallLuaCallbacks<void, luabind::object>("OnPlayerJoined", pl.GetLuaObject());
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnPlayerJoined(pl);
}

unsigned char pragma::Game::GetPlayerCount() { return m_numPlayers; }

LuaEntityManager &pragma::Game::GetLuaEntityManager() { return *m_luaEnts.get(); }

pragma::AnimationUpdateManager &pragma::Game::GetAnimationUpdateManager() { return *m_animUpdateManager; }

const pragma::game::GameModeInfo *pragma::Game::GetGameMode() const { return const_cast<Game *>(this)->GetGameMode(); }
pragma::game::GameModeInfo *pragma::Game::GetGameMode() { return m_gameMode; }
void pragma::Game::SetGameMode(const std::string &gameMode)
{
	auto *info = game::GameModeManager::GetGameModeInfo(gameMode);
	m_gameMode = info;

	if(info) {
		for(auto &pair : info->gameMountPriorities)
			util::set_mounted_game_priority(pair.first, pair.second);
	}
}

void pragma::Game::SetupEntity(ecs::BaseEntity *) {}

bool pragma::Game::IsMultiPlayer() const { return Engine::Get()->IsMultiPlayer(); }
bool pragma::Game::IsSinglePlayer() const { return Engine::Get()->IsSinglePlayer(); }

const pragma::physics::IEnvironment *pragma::Game::GetPhysicsEnvironment() const { return const_cast<Game *>(this)->GetPhysicsEnvironment(); }
pragma::physics::IEnvironment *pragma::Game::GetPhysicsEnvironment() { return m_physEnvironment.get(); }

void pragma::Game::OnEntityCreated(ecs::BaseEntity *ent)
{
	CallCallbacks<void, ecs::BaseEntity *>("OnEntityCreated", ent);
	auto &o = ent->GetLuaObject();
	CallLuaCallbacks<void, luabind::object>("OnEntityCreated", o);
}

Vector3 &pragma::Game::GetGravity() { return m_gravity; }
void pragma::Game::SetGravity(Vector3 &gravity) { m_gravity = gravity; }

std::vector<std::string> *pragma::Game::GetLuaNetMessageIndices() { return &m_luaNetMessageIndex; }

LuaDirectoryWatcherManager &pragma::Game::GetLuaScriptWatcher() { return *m_scriptWatcher; }
pragma::util::ResourceWatcherManager &pragma::Game::GetResourceWatcher() { return GetNetworkState()->GetResourceWatcher(); }

const std::shared_ptr<pragma::nav::Mesh> &pragma::Game::GetNavMesh() const { return const_cast<Game *>(this)->GetNavMesh(); }
std::shared_ptr<pragma::nav::Mesh> &pragma::Game::GetNavMesh() { return m_navMesh; }

std::shared_ptr<pragma::nav::Mesh> pragma::Game::LoadNavMesh(const std::string &fname) { return nav::Mesh::Load(*this, fname); }

bool pragma::Game::LoadNavMesh(bool bReload)
{
	if(m_navMesh != nullptr) {
		if(bReload == false)
			return true;
		m_navMesh = nullptr;
	}
	std::string path = "maps\\";
	path += GetMapName();

	auto pathAscii = path + "." + std::string {nav::PNAV_EXTENSION_ASCII};
	auto pathBinary = path + "." + std::string {nav::PNAV_EXTENSION_BINARY};
	path = fs::exists(pathBinary) ? pathBinary : pathAscii;
	Con::COUT << "Loading navigation mesh..." << Con::endl;

	m_navMesh = LoadNavMesh(path);
	if(m_navMesh == nullptr)
		Con::CWAR << "Unable to load navigation mesh!" << Con::endl;
	BaseAIComponent::ReloadNavThread(*this);
	return m_navMesh != nullptr;
}

void pragma::Game::InitializeMountedAddon(const AddonInfo &addonInfo) const { m_scriptWatcher->MountDirectory(addonInfo.GetLocalPath() + "/lua", true); }

void pragma::Game::Initialize()
{
	m_componentManager = InitializeEntityComponentManager();
	InitializeEntityComponents(*m_componentManager);

	m_animUpdateManager = std::make_unique<AnimationUpdateManager>(*this);

	InitializeLuaScriptWatcher();
	m_scriptWatcher->MountDirectory("lua");
	auto &addons = AddonSystem::GetMountedAddons();
	for(auto &info : addons)
		InitializeMountedAddon(info);

	LoadSoundScripts("fx.udm");
}

void pragma::Game::OnInitialized()
{
	auto &mdlManager = GetNetworkState()->GetModelManager();
	mdlManager.PreloadAsset("error");
}

void pragma::Game::SetUp() {}

static bool check_validity(pragma::BasePhysicsComponent &physC) { return !pragma::math::is_flag_set(physC.BaseEntityComponent::GetStateFlags(), pragma::BaseEntityComponent::StateFlags::Removed); }
static bool check_validity(pragma::ecs::BaseEntity *ent)
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

void pragma::Game::InitializeGame()
{
	InitializeLua(); // Lua has to be initialized completely before any entites are created

	auto physEngineName = GetConVarString("phys_engine");
	auto physEngineLibName = physics::IEnvironment::GetPhysicsEngineModuleLocation(physEngineName);
	spdlog::info("Loading physics module '{}'...", physEngineLibName);
	std::string err;
	auto dllHandle = GetNetworkState()->InitializeLibrary(physEngineLibName, &err);
	if(dllHandle) {
		auto *fInitPhysicsEngine = dllHandle->FindSymbolAddress<void (*)(NetworkState &, std::unique_ptr<physics::IEnvironment, void (*)(physics::IEnvironment *)> &)>("initialize_physics_engine");
		if(fInitPhysicsEngine != nullptr)
			fInitPhysicsEngine(*GetNetworkState(), m_physEnvironment);
		else
			Con::CERR << "Unable to initialize physics engine '" << physEngineName << "': Function 'initialize_physics_engine' not found!" << Con::endl;
	}
	else
		Con::CERR << "Unable to initialize physics engine '" << physEngineName << "': " << err << Con::endl;
	if(m_physEnvironment) {
		m_surfaceMaterialManager = std::make_unique<physics::SurfaceMaterialManager>(*m_physEnvironment);
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

	m_cbProfilingHandle = Engine::Get()->AddProfilingHandler([this](bool profilingEnabled) {
		if(profilingEnabled == false) {
			m_profilingStageManager = nullptr;
			return;
		}
		std::string postFix = IsClient() ? " (CL)" : " (SV)";
		auto &cpuProfiler = Engine::Get()->GetProfiler();
		m_profilingStageManager = std::make_unique<debug::ProfilingStageManager<debug::ProfilingStage>>();
		m_profilingStageManager->InitializeProfilingStageManager(cpuProfiler);
	});
}

void pragma::Game::GetEntities(std::vector<ecs::BaseEntity *> **ents) { *ents = &m_baseEnts; }
void pragma::Game::GetSpawnedEntities(std::vector<ecs::BaseEntity *> *ents)
{
	std::vector<ecs::BaseEntity *> *baseEnts;
	GetEntities(&baseEnts);
	for(unsigned int i = 0; i < baseEnts->size(); i++) {
		ecs::BaseEntity *ent = (*baseEnts)[i];
		if(ent != nullptr && ent->IsSpawned())
			ents->push_back(ent);
	}
}

pragma::NetworkState *pragma::Game::GetNetworkState() { return m_stateNetwork; }

void pragma::Game::UpdateTime()
{
	float timeScale = GetTimeScale();
	m_ctCur.Update(timeScale);
	m_ctReal.Update();
	m_tCur = CDouble(m_ctCur());
	m_tReal = CDouble(m_ctReal());
	m_tDelta = CDouble(m_tCur - m_tLast);
	m_tDeltaReal = CDouble(m_tReal - m_tLastReal);
}
void pragma::Game::Think()
{
	UpdateTime();

	m_scriptWatcher->Poll(); // TODO: Don't do this every frame?
}
void pragma::Game::PostThink()
{
	m_tLast = m_tCur;
	m_tLastReal = m_tReal;
}

double &pragma::Game::GetLastThink() { return m_tLast; }
double &pragma::Game::GetLastTick() { return m_tLastTick; }

pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *pragma::Game::GetProfilingStageManager() { return m_profilingStageManager.get(); }
bool pragma::Game::StartProfilingStage(const char *stage)
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("stage_" + std::string {stage});
#endif
	return m_profilingStageManager && m_profilingStageManager->StartProfilerStage(stage);
}
bool pragma::Game::StopProfilingStage()
{
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().EndTask();
#endif
	return m_profilingStageManager && m_profilingStageManager->StopProfilerStage();
}

std::string pragma::Game::GetMapName() { return m_mapInfo.name; }

const std::vector<pragma::ecs::BaseEntity *> &pragma::Game::GetBaseEntities() const { return const_cast<Game *>(this)->GetBaseEntities(); }
std::vector<pragma::ecs::BaseEntity *> &pragma::Game::GetBaseEntities() { return m_baseEnts; }
std::size_t pragma::Game::GetBaseEntityCount() const { return m_numEnts; }

void pragma::Game::ScheduleEntityForRemoval(ecs::BaseEntity &ent) { m_entsScheduledForRemoval.push(ent.GetHandle()); }

void pragma::Game::UpdateAnimations(double dt) { m_animUpdateManager->UpdateAnimations(dt); }

void pragma::Game::Tick()
{
	StartProfilingStage("Tick");
	if((m_flags & GameFlags::InitialTick) != GameFlags::None) {
		m_flags &= ~GameFlags::InitialTick;
		m_tDeltaTick = 0.0f; // First tick is essentially 'skipped' to avoid physics errors after the world has been loaded
	}
	else
		m_tDeltaTick = (1.f / Engine::Get()->GetTickRate()) * GetTimeScale(); //m_tCur -m_tLastTick;
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
		if(hPhysC.expired() || hPhysC->GetPhysicsType() == physics::PhysicsType::None)
			continue;
		hPhysC->PrePhysicsSimulate(); // Has to be called BEFORE PhysicsUpdate (This is where stuff like Character movement is handled)!
	}

	for(auto &hPhysC : awakePhysics) {
		if(hPhysC.expired() || hPhysC->GetPhysicsType() == physics::PhysicsType::None)
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
		if(hPhysC.expired() || hPhysC->GetPhysicsType() == physics::PhysicsType::None) {
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
	BaseEntityComponentSystem::Cleanup();

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
void pragma::Game::PostTick() { m_tLastTick = m_tCur; }

void pragma::Game::SetGameFlags(GameFlags flags) { m_flags = flags; }
pragma::Game::GameFlags pragma::Game::GetGameFlags() const { return m_flags; }

bool pragma::Game::IsMapInitialized() const { return (m_flags & GameFlags::MapInitialized) != GameFlags::None; }
bool pragma::Game::IsGameReady() const { return (m_flags & GameFlags::GameReady) != GameFlags::None; }

const MapInfo &pragma::Game::GetMapInfo() const { return m_mapInfo; }

void pragma::Game::UpdatePackagePaths()
{
	std::vector<std::string> packagePaths = {};
	auto &addons = AddonSystem::GetMountedAddons();
	packagePaths.reserve(2 + addons.size());

	for(auto &path : fs::get_absolute_root_paths()) {
		packagePaths.push_back(pragma::util::FilePath(path, "lua/?.lua").GetString());
		packagePaths.push_back(pragma::util::FilePath(path, "lua/modules/?.lua").GetString());
	}

	for(auto &addonInfo : addons) {
		auto path = util::Path::CreatePath(addonInfo.GetAbsolutePath()) + "lua/modules/?.lua";
		packagePaths.push_back(path.GetString());
	}
	auto package = luabind::object {luabind::globals(GetLuaState())["package"]};
	if(Lua::GetType(package) == Lua::Type::Nil)
		return;
	package["path"] = string::implode(packagePaths, ";");

#ifdef _WIN32
	std::string ext = ".dll";
#else
	std::string ext = ".so";
#endif
	auto path = util::Path::CreatePath(util::get_program_path());
	package["cpath"] = pragma::util::FilePath(path, ("modules/?" + ext)).GetString();
}

bool pragma::Game::LoadSoundScripts(const char *file) { return m_stateNetwork->LoadSoundScripts(file, true); }
bool pragma::Game::LoadMap(const std::string &map, const Vector3 &origin, std::vector<EntityHandle> *entities)
{
	auto normPath = pragma::asset::get_normalized_path(map, asset::Type::Map);
	std::string format;
	auto filePath = pragma::asset::find_file(normPath, asset::Type::Map, &format);
	if(filePath.has_value() == false) {
		static auto bPort = true;
		if(bPort == true) {
			Con::CWAR << "Map '" << map << "' not found." << Con::endl;
			auto path = pragma::asset::relative_path_to_absolute_path(normPath, asset::Type::Map);
			if(pragma::util::port_source2_map(GetNetworkState(), path.GetString()) == false && pragma::util::port_hl2_map(GetNetworkState(), path.GetString()) == false)
				Con::CWAR << " Loading empty map..." << Con::endl;
			else {
				Con::CWAR << Con::endl;
				Con::COUT << "Successfully ported HL2 map " << path.GetString() << "!" << Con::endl;
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
	m_mapInfo.fileName = pragma::asset::relative_path_to_absolute_path(*filePath, asset::Type::Map).GetString();
	util::ScopeGuard sg {[this]() { m_flags |= GameFlags::MapInitialized; }};

	auto error = [this, &map](const std::string_view &msg) { Con::CWAR << "Unable to load map '" << map << "': " << msg << Con::endl; };

	auto f = pragma::fs::open_file(m_mapInfo.fileName.c_str(), fs::FileMode::Read | fs::FileMode::Binary);
	if(f == nullptr) {
		error("Unable to open file '" + m_mapInfo.fileName + "'!");
		return false;
	}

	auto worldData = asset::WorldData::Create(*GetNetworkState());
	if(asset::matches_format(format, asset::FORMAT_MAP_BINARY) || asset::matches_format(format, asset::FORMAT_MAP_ASCII)) {
		auto udmData = util::load_udm_asset(std::make_unique<fs::File>(f));
		std::string err;
		if(udmData == nullptr || worldData->LoadFromAssetData(udmData->GetAssetData(), asset::EntityData::Flags::None, err) == false) {
			error(err);
			return false;
		}
	}
	else if(asset::matches_format(format, asset::FORMAT_MAP_LEGACY)) {
		std::string err;
		auto success = worldData->Read(f, asset::EntityData::Flags::None, &err);
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
	Con::COUT << "Loading entities..." << Con::endl;

	BaseStaticBvhCacheComponent *bvhC = nullptr;
	if(IsClient()) {
		auto *entBvh = CreateEntity("entity");
		assert(entBvh);
		bvhC = static_cast<BaseStaticBvhCacheComponent *>(entBvh->AddComponent("static_bvh_cache").get());
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

void pragma::Game::InitializeWorldData(asset::WorldData &worldData) {}
void pragma::Game::InitializeMapEntities(asset::WorldData &worldData, std::vector<EntityHandle> &outEnt)
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

std::shared_ptr<pragma::asset::Model> pragma::Game::CreateModel(const std::string &mdl) const { return m_stateNetwork->GetModelManager().CreateModel(mdl); }
std::shared_ptr<pragma::asset::Model> pragma::Game::CreateModel(bool bAddReference) const { return m_stateNetwork->GetModelManager().CreateModel("", bAddReference); }
bool pragma::Game::PrecacheModel(const std::string &mdl)
{
	spdlog::info("Precaching model '{}'...", mdl);
	auto *asset = GetNetworkState()->GetModelManager().FindCachedAsset(mdl);
	if(asset)
		return true;
	auto loadInfo = std::make_unique<asset::ModelLoadInfo>();
	auto r = GetNetworkState()->GetModelManager().PreloadAsset(mdl, std::move(loadInfo));
	if(r.assetRequest) {
		r.assetRequest->AddCallback([this](util::Asset *asset, util::AssetLoadResult result) {
			if (result != util::AssetLoadResult::Succeeded)
				return;
			assert(asset != nullptr);
			auto mdl = asset::ModelManager::GetAssetObject(*asset);
			CallCallbacks<void, std::reference_wrapper<std::shared_ptr<asset::Model>>>("OnModelLoaded", mdl);
			CallLuaCallbacks<void, std::shared_ptr<asset::Model>>("OnModelLoaded", mdl);
		});
	}
	return r;
}
std::shared_ptr<pragma::asset::Model> pragma::Game::LoadModel(const std::string &mdl, bool bReload)
{
	if(mdl.empty())
		return nullptr;
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	::debug::get_domain().BeginTask("load_model");
	pragma::util::ScopeGuard sgVtune {[]() { ::debug::get_domain().EndTask(); }};
#endif
	spdlog::debug("Loading model '{}'...", mdl);
	auto *asset = GetNetworkState()->GetModelManager().FindCachedAsset(mdl);
	if(asset)
		return asset::ModelManager::GetAssetObject(*asset);
	auto &mdlMananger = GetNetworkState()->GetModelManager();
	util::FileAssetManager::PreloadResult result;
	auto r = bReload ? mdlMananger.ReloadAsset(mdl, nullptr, &result) : mdlMananger.LoadAsset(mdl, nullptr, &result);
	if(r != nullptr) {
		CallCallbacks<void, std::reference_wrapper<std::shared_ptr<asset::Model>>>("OnModelLoaded", r);
		CallLuaCallbacks<void, std::shared_ptr<asset::Model>>("OnModelLoaded", r);
	}
	else {
		std::string errMsg = result.errorMessage ? *result.errorMessage : "Unknown error";
		Con::CWAR << "Failed to load model '" << mdl << "': " << errMsg << Con::endl;
	}
	return r;
}

void pragma::Game::OnGameReady()
{
	// All assets have been loaded, now clear the unused assets (if there was a previous map)
	Con::COUT << "Clearing unused assets..." << Con::endl;
	auto numModelsCleared = GetNetworkState()->GetModelManager().ClearUnused();
	auto numMaterialsCleared = GetNetworkState()->GetMaterialManager().ClearUnused();
	Con::COUT << numModelsCleared << " models and " << numMaterialsCleared << " materials have been cleared from cache!" << Con::endl;

	m_ctCur.Reset();
	m_ctReal.Reset();
	CallCallbacks<void>("OnGameReady");

	m_flags |= GameFlags::GameReady;
	CallLuaCallbacks("OnGameReady");
	for(auto *gmC : GetGamemodeComponents())
		gmC->OnGameReady();
}

void pragma::Game::SetWorld(BaseWorldComponent *entWorld)
{
	if(!entWorld)
		return;
	m_worldComponents.push_back(entWorld->GetHandle<BaseWorldComponent>());
}

std::vector<pragma::ComponentHandle<pragma::BasePhysicsComponent>> &pragma::Game::GetAwakePhysicsComponents() { return m_awakePhysicsEntities; }

const pragma::EntityComponentManager &pragma::Game::GetEntityComponentManager() const { return const_cast<Game *>(this)->GetEntityComponentManager(); }
pragma::EntityComponentManager &pragma::Game::GetEntityComponentManager() { return *m_componentManager; }

pragma::physics::SurfaceMaterial &pragma::Game::CreateSurfaceMaterial(const std::string &identifier, Float friction, Float restitution) { return m_surfaceMaterialManager->Create(identifier, friction, restitution); }
pragma::physics::SurfaceMaterial *pragma::Game::GetSurfaceMaterial(const std::string &id) { return m_surfaceMaterialManager ? m_surfaceMaterialManager->GetMaterial(id) : nullptr; }
pragma::physics::SurfaceMaterial *pragma::Game::GetSurfaceMaterial(UInt32 id)
{
	if(m_surfaceMaterialManager == nullptr)
		return nullptr;
	auto &materials = m_surfaceMaterialManager->GetMaterials();
	if(id >= materials.size())
		return nullptr;
	return &materials[id];
}
std::vector<pragma::physics::SurfaceMaterial> *pragma::Game::GetSurfaceMaterials() { return m_surfaceMaterialManager ? &m_surfaceMaterialManager->GetMaterials() : nullptr; }

double &pragma::Game::RealTime() { return m_tReal; }
double &pragma::Game::CurTime() { return m_tCur; }
double &pragma::Game::ServerTime() { return CurTime(); }
double &pragma::Game::DeltaTime() { return m_tDelta; }
double &pragma::Game::DeltaRealTime() { return m_stateNetwork->DeltaTime(); }
double &pragma::Game::LastThink() { return m_tLast; }
double &pragma::Game::LastTick() { return m_tLastTick; }
double &pragma::Game::DeltaTickTime() { return m_tDeltaTick; }

float pragma::Game::GetTimeScale() { return 1.f; }
void pragma::Game::SetTimeScale(float t) { m_stateNetwork->SetConVar("host_timescale", std::to_string(t)); }

pragma::console::ConConf *pragma::Game::GetConVar(const std::string &scmd) { return m_stateNetwork->GetConVar(scmd); }
int pragma::Game::GetConVarInt(const std::string &scmd) { return m_stateNetwork->GetConVarInt(scmd); }
std::string pragma::Game::GetConVarString(const std::string &scmd) { return m_stateNetwork->GetConVarString(scmd); }
float pragma::Game::GetConVarFloat(const std::string &scmd) { return m_stateNetwork->GetConVarFloat(scmd); }
bool pragma::Game::GetConVarBool(const std::string &scmd) { return m_stateNetwork->GetConVarBool(scmd); }
pragma::console::ConVarFlags pragma::Game::GetConVarFlags(const std::string &scmd) { return m_stateNetwork->GetConVarFlags(scmd); }
