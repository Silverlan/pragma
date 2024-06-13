/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __GAME_H__
#define __GAME_H__
#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <pragma/lua/luaapi.h>
#include "pragma/lua/ldefinitions.h"
#include "pragma/level/mapinfo.h"
#include "pragma/lua/d_hooks.h"
#include <vector>
#include <deque>
#include "pragma/physics/physicstypes.h"
#include "pragma/entities/baseentity_handle.h"
#include <sharedutils/callback_handler.h>
#include "pragma/lua/lua_callback_handler.h"
#include "pragma/lua/lua_script_watcher.h"
#include <sharedutils/chronotime.h>
#include "pragma/util/timertypes.h"
#include <fsys/vfileptr.h>
#include "pragma/lua/sh_lua_entity_manager.h"
#include "pragma/util/ammo_type.h"
#include "pragma/math/surfacematerial.h"
#include "pragma/entities/baseentity_net_event_manager.hpp"
#include "pragma/console/cvar_callback.hpp"
#include <fsys/filesystem.h>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/util_shared_handle.hpp>
#include <pragma/console/fcvar.h>
#include <pragma/debug/debug_performance_profiler.hpp>
#ifdef __linux__
#include "pragma/lua/lua_script_watcher.h"
#include "pragma/physics/environment.hpp"
#endif

namespace Lua {
	DLLNETWORK void StackDump(lua_State *lua);
	DLLNETWORK std::optional<std::string> StackToString(lua_State *lua);
	DLLNETWORK std::optional<std::string> TableToString(lua_State *lua, int n = -1);
	DLLNETWORK void TableDump(lua_State *lua, int n = -1);
	DLLNETWORK void VarDump(lua_State *lua, int n = -1);
	DLLNETWORK std::optional<std::string> VarToString(lua_State *lua, int n = -1);
	class Interface;
}

#undef DrawText

class AddonInfo;
class BrushMesh;
class Side;
class NetworkState;
class ConConf;
struct TraceResult;
class BaseEntity;
class WVPxEventCallback;
class PhysObj;
class TCallback;
class SurfaceMaterial;
class TraceData;
class Timer;
class Model;
class ModelMesh;
class ModelSubMesh;
class LuaDirectoryWatcherManager;
class ResourceWatcherManager;
struct AmmoType;
struct GameModeInfo;
struct Color;
class GibletCreateInfo;
enum class RayCastFlags : uint32_t;
namespace pragma {
	using ComponentId = uint32_t;
	class BaseWorldComponent;
	class BaseEntityComponent;
	class BasePhysicsComponent;
	class EntityComponentManager;
	class BasePlayerComponent;
	class BaseGamemodeComponent;
	class BaseGameComponent;
	struct AnimationUpdateManager;
	namespace nav {
		class Mesh;
	};
	namespace asset {
		class WorldData;
		class EntityData;
	};
	namespace lua {
		class ClassManager;
	};
	namespace networking {
		enum class DropReason : int8_t;
	};
};

namespace util {
	using Uuid = std::array<uint64_t, 2>;
};

struct BaseEntityComponentHandleWrapper;
namespace pragma::physics {
	class IEnvironment;
};
class DLLNETWORK Game : public CallbackHandler, public LuaCallbackHandler {
  public:
	pragma::physics::IEnvironment *GetPhysicsEnvironment();
	const pragma::physics::IEnvironment *GetPhysicsEnvironment() const;
	SurfaceMaterial &CreateSurfaceMaterial(const std::string &identifier, Float friction = 0.5f, Float restitution = 0.5f);
	SurfaceMaterial *GetSurfaceMaterial(const std::string &id);
	SurfaceMaterial *GetSurfaceMaterial(UInt32 id);
	std::vector<SurfaceMaterial> &GetSurfaceMaterials();

	enum class GameFlags : uint32_t {
		None = 0u,
		MapInitialized = 1u,
		GameInitialized = MapInitialized << 1u,
		MapLoaded = GameInitialized << 1u,
		InitialTick = MapLoaded << 1u,
		LevelTransition = InitialTick << 1u,
		ClosingGame = LevelTransition << 1u,
		GameReady = ClosingGame << 1u,
	};

	Vector3 &GetGravity();
	void SetGravity(Vector3 &gravity);
	std::shared_ptr<Model> CreateModel(const std::string &mdl) const;
	std::shared_ptr<Model> CreateModel(bool bAddReference = true) const;
	std::shared_ptr<Model> LoadModel(const std::string &mdl, bool bReload = false);
	bool PrecacheModel(const std::string &mdl);
	bool RunLua(const std::string &lua, const std::string &chunkName);
	LuaDirectoryWatcherManager &GetLuaScriptWatcher();
	ResourceWatcherManager &GetResourceWatcher();
	void ReloadGameModeScripts();

	virtual std::shared_ptr<ModelMesh> CreateModelMesh() const = 0;
	virtual std::shared_ptr<ModelSubMesh> CreateModelSubMesh() const = 0;

	virtual void GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const = 0;
	void ScheduleEntityForRemoval(BaseEntity &ent);

	virtual pragma::NetEventId FindNetEvent(const std::string &name) const = 0;
	virtual pragma::NetEventId SetupNetEvent(const std::string &name) = 0;

	bool IsGameModeInitialized() const;
	bool IsGameInitialized() const;
	bool IsMapLoaded() const;
	const std::array<std::string, 6> &GetLuaEntityDirectories() const;
	virtual void InitializeGame();
	virtual void InitializeLua();
	virtual void SetupLua();
	virtual void SetUp();
	virtual void Initialize();
	virtual void OnInitialized();
	void InitializeMountedAddon(const AddonInfo &addonInfo) const;

	const pragma::EntityComponentManager &GetEntityComponentManager() const;
	pragma::EntityComponentManager &GetEntityComponentManager();

	// Entities
	const std::vector<BaseEntity *> &GetBaseEntities() const;
	std::vector<BaseEntity *> &GetBaseEntities();
	std::size_t GetBaseEntityCount() const;
	virtual void GetEntities(std::vector<BaseEntity *> **ents);
	void GetSpawnedEntities(std::vector<BaseEntity *> *ents);

	virtual void GetPlayers(std::vector<BaseEntity *> *ents) = 0;
	virtual void GetNPCs(std::vector<BaseEntity *> *ents) = 0;
	virtual void GetWeapons(std::vector<BaseEntity *> *ents) = 0;
	virtual void GetVehicles(std::vector<BaseEntity *> *ents) = 0;

	virtual void GetPlayers(std::vector<EntityHandle> *ents) = 0;
	virtual void GetNPCs(std::vector<EntityHandle> *ents) = 0;
	virtual void GetWeapons(std::vector<EntityHandle> *ents) = 0;
	virtual void GetVehicles(std::vector<EntityHandle> *ents) = 0;

	virtual BaseEntity *CreateEntity();
	virtual BaseEntity *CreateEntity(std::string classname);
	virtual void RemoveEntity(BaseEntity *ent);
	void RemoveEntities();
	virtual BaseEntity *GetEntity(unsigned int idx);
	virtual BaseEntity *GetEntityByLocalIndex(uint32_t idx);
	const BaseEntity *FindEntityByUniqueId(const util::Uuid &uuid) const { return const_cast<Game *>(this)->FindEntityByUniqueId(uuid); }
	BaseEntity *FindEntityByUniqueId(const util::Uuid &uuid);
	const std::unordered_map<size_t, BaseEntity *> &GetEntityUuidMap() const { return const_cast<Game *>(this)->GetEntityUuidMap(); }
	std::unordered_map<size_t, BaseEntity *> &GetEntityUuidMap() { return m_uuidToEnt; }
	pragma::BaseWorldComponent *GetWorld();
	const std::vector<util::TWeakSharedHandle<pragma::BaseWorldComponent>> &GetWorldComponents() const;
	unsigned char GetPlayerCount();
	unsigned int GetEntityCount();
	virtual void SpawnEntity(BaseEntity *ent);
	void SplashDamage(const Vector3 &origin, Float radius, DamageInfo &dmg, const std::function<bool(BaseEntity *, DamageInfo &)> &callback = nullptr);
	void SplashDamage(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, BaseEntity *attacker = nullptr, BaseEntity *inflictor = nullptr, const std::function<bool(BaseEntity *, DamageInfo &)> &callback = nullptr);
	void SplashDamage(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, const EntityHandle &attacker = EntityHandle(), const EntityHandle &inflictor = EntityHandle(), const std::function<bool(BaseEntity *, DamageInfo &)> &callback = nullptr);

	Bool Overlap(const TraceData &data, std::vector<TraceResult> *optOutResults) const;
	Bool RayCast(const TraceData &data, std::vector<TraceResult> *optOutResults) const;
	Bool Sweep(const TraceData &data, std::vector<TraceResult> *optOutResults) const;

	TraceResult Overlap(const TraceData &data) const;
	TraceResult RayCast(const TraceData &data) const;
	TraceResult Sweep(const TraceData &data) const;

	virtual void CreateGiblet(const GibletCreateInfo &info) = 0;

	const std::shared_ptr<pragma::nav::Mesh> &GetNavMesh() const;
	std::shared_ptr<pragma::nav::Mesh> &GetNavMesh();

	bool IsMultiPlayer() const;
	bool IsSinglePlayer() const;
	// Map
	bool IsMapInitialized() const;
	bool IsGameReady() const;
	template<class TWorld, class TPolyMesh, class TPoly, class TBrushMesh>
	void BuildVMF(const char *map);
	virtual bool LoadMap(const std::string &map, const Vector3 &origin = {}, std::vector<EntityHandle> *entities = nullptr);
	// Called when map and gamemode has been fully loaded and the game can start proper
	virtual void OnGameReady();
	bool LoadNavMesh(bool bReload = false);
	AmmoTypeManager &GetAmmoTypeManager();
	Bool RegisterAmmoType(const std::string &name, Int32 damage = 10, Float force = 200.f, DAMAGETYPE dmgType = DAMAGETYPE::BULLET, AmmoType **ammoOut = nullptr);
	AmmoType *GetAmmoType(const std::string &name, UInt32 *ammoId = nullptr);
	AmmoType *GetAmmoType(UInt32 ammoId);
	const GameModeInfo *GetGameMode() const;
	GameModeInfo *GetGameMode();
	void SetGameMode(const std::string &gameMode);
	BaseEntity *GetGameModeEntity();
	BaseEntity *GetGameEntity();
	LuaEntityManager &GetLuaEntityManager();
	const LuaEntityManager &GetLuaEntityManager() const { return const_cast<Game *>(this)->GetLuaEntityManager(); }
	pragma::AnimationUpdateManager &GetAnimationUpdateManager();
	void UpdatePackagePaths();

	void SetWorld(pragma::BaseWorldComponent *entWorld);
	void CloseMap();
	void ClearModels();
	virtual void Think();
	virtual void Tick();
	void PostThink();
	void PostTick();
	void UpdateAnimations(double dt);
	std::string GetMapName();
	void PrecacheModel(const char *mdl);
	bool LoadSoundScripts(const char *file);
	std::string GetSoundScript(const char *script);
	bool ExecConfig(std::string cfg);
	// Collision Detection
	bool m_bCollisionsEnabled = true;
	bool CollisionTest();
	bool CollisionTest(Vector3 *pos);
	bool CollisionTest(pragma::BasePlayerComponent &pl, float *distance, Vector3 *hitnormal = NULL);
	void CollisionTest(BaseEntity *a, BaseEntity *b);
	void EnableCollisions(bool b);
	//
  public:
	//
	enum class CPUProfilingPhase : uint32_t {
		Tick = 0u,
		Physics,
		PhysicsSimulation,
		GameObjectLogic,
		Timers,
		Animations,

		Count
	};
	Game(NetworkState *state);
	virtual ~Game();
	virtual void OnRemove();
	virtual bool IsServer();
	virtual bool IsClient();
	NetworkState *GetNetworkState();
	// Time
	double &LastThink();
	double &LastTick();
	virtual double &CurTime();
	virtual double &ServerTime();
	virtual double &RealTime();
	virtual double &DeltaRealTime();
	// This is the frame delta time! For tick time, use DeltaTickTime!
	virtual double &DeltaTime();
	virtual double &DeltaTickTime();
	double &GetLastTick();
	double &GetLastThink();
	virtual float GetTimeScale();
	virtual void SetTimeScale(float t);
	// Lua
	lua_State *GetLuaState();
	Lua::Interface &GetLuaInterface();
	virtual void RegisterLua();
	virtual void RegisterLuaGlobals();
	virtual void RegisterLuaClasses();
	void RegisterLuaGameClasses(luabind::module_ &gameMod);
	virtual void RegisterLuaLibraries();
	virtual bool RegisterNetMessage(std::string name);
	void RegisterLuaNetMessage(std::string name, int handler);
	std::vector<std::string> *GetLuaNetMessageIndices();
	bool BroadcastEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx);
	bool InjectEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx);
	Lua::StatusCode LoadLuaFile(std::string &fInOut, fsys::SearchFlags includeFlags = fsys::SearchFlags::All, fsys::SearchFlags excludeFlags = fsys::SearchFlags::None);
	virtual bool ExecuteLuaFile(std::string &fInOut, lua_State *optCustomLuaState = nullptr);
	// Same as ExecuteLuaFile, but uses the last value from the include stack
	//bool IncludeLuaFile(std::string &fInOut); // Deprecated

	virtual bool RunLua(const std::string &lua) = 0;
	virtual void RunLuaFiles(const std::string &subPath);
	Lua::StatusCode ProtectedLuaCall(const std::function<Lua::StatusCode(lua_State *)> &pushFuncArgs, int32_t numResults);
	template<class TLuaEntity, class THandle>
	BaseEntity *CreateLuaEntity(std::string classname, luabind::object &oClass, bool bLoadIfNotExists = false);
	virtual BaseEntity *CreateLuaEntity(std::string classname, bool bLoadIfNotExists = false) = 0;
	virtual pragma::BaseEntityComponent *CreateLuaEntityComponent(BaseEntity &ent, std::string classname) = 0;
	void LoadLuaEntities(std::string subPath);
	void LoadLuaComponents(const std::string &typePath);
	virtual std::string GetLuaNetworkDirectoryName() const = 0;
	virtual std::string GetLuaNetworkFileName() const = 0;
	std::string GetGameModeScriptDirectoryPath() const;
	std::string GetGameModeScriptDirectoryNetworkPath() const;
	std::string GetGameModeScriptFilePath() const;

	// Path should be path to directory or file of lua entity (relative to "lua" directory)
	bool LoadLuaEntity(std::string path);
	// Path should be path to directory or file of lua component (relative to "lua" directory)
	bool LoadLuaComponent(std::string path);

	bool LoadLuaEntity(const std::string &mainPath, const std::string &className);
	bool LoadLuaComponent(const std::string &mainPath, const std::string &componentName);
	bool LoadLuaEntityByClass(const std::string &className);
	bool LoadLuaComponentByName(const std::string &componentName);
	const pragma::lua::ClassManager &GetLuaClassManager() const;
	pragma::lua::ClassManager &GetLuaClassManager();

	CallbackHandle AddConVarCallback(const std::string &cvar, LuaFunction function);
	unsigned int GetNetMessageID(std::string name);
	std::string *GetNetMessageIdentifier(unsigned int ID);
	virtual void OnPlayerDropped(pragma::BasePlayerComponent &pl, pragma::networking::DropReason reason);
	virtual void OnPlayerReady(pragma::BasePlayerComponent &pl);
	virtual void OnPlayerJoined(pragma::BasePlayerComponent &pl);
	// Timers
	Timer *CreateTimer(float delay, int reps, LuaFunctionObject luaFunction, TimerType timeType = TimerType::CurTime);
	Timer *CreateTimer(float delay, int reps, const CallbackHandle &hCallback, TimerType timeType = TimerType::CurTime);
	void ClearTimers();
	// ConVars
	template<class T>
	T *GetConVar(const std::string &scmd);
	ConConf *GetConVar(const std::string &scmd);
	int GetConVarInt(const std::string &scmd);
	std::string GetConVarString(const std::string &scmd);
	float GetConVarFloat(const std::string &scmd);
	bool GetConVarBool(const std::string &scmd);
	ConVarFlags GetConVarFlags(const std::string &scmd);
	const std::unordered_map<std::string, std::vector<CvarCallback>> &GetConVarCallbacks() const;

	virtual Float GetFrictionScale() const = 0;
	virtual Float GetRestitutionScale() const = 0;
	const MapInfo &GetMapInfo() const;

	void SetGameFlags(GameFlags flags);
	GameFlags GetGameFlags() const;

	virtual bool IsPhysicsSimulationEnabled() const = 0;

	std::vector<pragma::ComponentHandle<pragma::BasePhysicsComponent>> &GetAwakePhysicsComponents();
	std::vector<pragma::BaseEntityComponent *> &GetEntityTickComponents() { return m_entityTickComponents; }
	std::vector<pragma::BaseGamemodeComponent *> &GetGamemodeComponents() { return m_gamemodeComponents; }

	// Debug
	virtual void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration = 0.f) = 0;
	virtual void DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration = 0.f) = 0;
	virtual void DrawPlane(const Vector3 &n, float dist, const Color &color, float duration = 0.f) = 0;
	virtual void DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration = 0.f) = 0;
	pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage, CPUProfilingPhase> *GetProfilingStageManager();
	bool StartProfilingStage(CPUProfilingPhase stage);
	bool StopProfilingStage(CPUProfilingPhase stage);
  protected:
	virtual void UpdateTime();
	void GetLuaRegisteredEntities(std::vector<std::string> &luaClasses) const;

	GameFlags m_flags = GameFlags::InitialTick;
	std::unique_ptr<pragma::AnimationUpdateManager> m_animUpdateManager;
	std::vector<BaseEntity *> m_baseEnts;
	std::unordered_map<size_t, BaseEntity *> m_uuidToEnt;
	std::queue<EntityHandle> m_entsScheduledForRemoval;
	std::vector<pragma::ComponentHandle<pragma::BasePhysicsComponent>> m_awakePhysicsEntities;
	std::vector<pragma::BaseEntityComponent *> m_entityTickComponents;
	std::vector<pragma::BaseGamemodeComponent *> m_gamemodeComponents;
	std::shared_ptr<Lua::Interface> m_lua = nullptr;
	std::unique_ptr<pragma::lua::ClassManager> m_luaClassManager;
	std::unique_ptr<LuaDirectoryWatcherManager> m_scriptWatcher = nullptr;
	std::unique_ptr<SurfaceMaterialManager> m_surfaceMaterialManager = nullptr;
	std::unordered_map<std::string, std::vector<CvarCallback>> m_cvarCallbacks;
	std::vector<std::unique_ptr<Timer>> m_timers;
	std::unordered_map<std::string, int> m_luaNetMessages;
	std::vector<std::string> m_luaNetMessageIndex;
	MapInfo m_mapInfo = {};
	std::deque<unsigned int> m_entIndices;
	uint32_t m_numEnts = 0u;
	uint8_t m_numPlayers = 0u;
	NetworkState *m_stateNetwork = nullptr;
	ChronoTime m_ctCur = {};
	ChronoTime m_ctReal = {};
	double m_tCur = 0.0;
	double m_tLast = 0.0;
	double m_tDelta = 0.0;
	double m_tReal = 0.0;
	double m_tLastTick = 0.0;
	double m_tDeltaTick = 0.0;
	double m_tDeltaReal = 0.0;
	double m_tLastReal = 0.0;
	// Physics have a fixed time-step, if the game delta time
	// doesn't match that time-step, the remainder will be used
	// for the next tick.
	float m_tPhysDeltaRemainder = 0.f;
	Vector3 m_gravity = {0, -600, 0};
	std::vector<util::TWeakSharedHandle<pragma::BaseWorldComponent>> m_worldComponents {};
	GameModeInfo *m_gameMode = nullptr;
	EntityHandle m_entGamemode;
	EntityHandle m_entGame;
	CallbackHandle m_cbProfilingHandle = {};
	std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage, CPUProfilingPhase>> m_profilingStageManager = nullptr;
	std::shared_ptr<pragma::nav::Mesh> m_navMesh = nullptr;
	std::unique_ptr<AmmoTypeManager> m_ammoTypes = nullptr;
	std::unique_ptr<LuaEntityManager> m_luaEnts = nullptr;
	std::shared_ptr<pragma::EntityComponentManager> m_componentManager = nullptr;

	// Lua
	std::vector<std::string> m_luaIncludeStack = {};

	virtual void InitializeWorldData(pragma::asset::WorldData &worldData);
	virtual void InitializeMapEntities(pragma::asset::WorldData &worldData, std::vector<EntityHandle> &outEnts);
	virtual bool LoadLuaComponent(const std::string &luaFilePath, const std::string &mainPath, const std::string &componentName);
	virtual bool InitializeGameMode();
	template<class TComponent, class THolder>
	pragma::BaseEntityComponent *CreateLuaEntityComponent(BaseEntity &ent, std::string classname);

	virtual bool InvokeEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject);
	virtual void RegisterLuaEntityComponents(luabind::module_ &gameMod);
	virtual void RegisterLuaEntityComponent(luabind::class_<pragma::BaseEntityComponent> &classDef);
	void LoadConfig();
	void SaveConfig();
	void UpdateTimers();
	virtual void InitializeLuaScriptWatcher();

	// Map
	BaseEntity *CreateMapEntity(pragma::asset::EntityData &entData);
	std::unique_ptr<pragma::physics::IEnvironment, void (*)(pragma::physics::IEnvironment *)> m_physEnvironment = std::unique_ptr<pragma::physics::IEnvironment, void (*)(pragma::physics::IEnvironment *)> {nullptr, [](pragma::physics::IEnvironment *) {}};

	virtual std::shared_ptr<pragma::EntityComponentManager> InitializeEntityComponentManager() = 0;
	virtual void OnEntityCreated(BaseEntity *ent);
	virtual void SetupEntity(BaseEntity *ent, unsigned int idx) = 0;
	virtual unsigned int GetFreeEntityIndex() = 0;
	virtual std::shared_ptr<pragma::nav::Mesh> LoadNavMesh(const std::string &fname);
	void SetupEntity(BaseEntity *ent);
	virtual void InitializeEntityComponents(pragma::EntityComponentManager &componentManager);
	virtual void OnMapLoaded();
};
REGISTER_BASIC_BITWISE_OPERATORS(Game::GameFlags)

template<class T>
T *Game::GetConVar(const std::string &scmd)
{
	ConConf *cv = GetConVar(scmd);
	if(cv == NULL)
		return NULL;
	return static_cast<T *>(cv);
}

DLLNETWORK void IncludeLuaEntityBaseClasses(lua_State *l, int refEntities, int obj, int data);

#endif
