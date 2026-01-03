// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"
#include "encryption/md5.h"

export module pragma.shared:game.game;

export import :console.convar;
export import :console.output;
export import :core.addon_system;
export import :debug.performance_profiler;
export import :entities.base_entity;
export import :entities.components.base;
export import :entities.components.base_gamemode;
export import :entities.components.base_physics;
export import :entities.components.base_world;
export import :entities.enums;
export import :game.ammo_type;
export import :game.animation_update_manager;
export import :map.map_info;
export import :map.world_data;
export import :physics.surface_material;
export import :scripting.lua.callback_handler;
export import :scripting.lua.class_manager;
export import :scripting.lua.entity_manager;
export import :scripting.lua.script_watcher;
export import :physics.raycast;
export import :types;
export import :util.data_file;
export import :util.giblet_create_info;
export import :util.resource_watcher;
export import :util.timer;

export {
	namespace pragma {
		class DLLNETWORK Game : public util::CallbackHandler, public LuaCallbackHandler {
		  public:
			physics::IEnvironment *GetPhysicsEnvironment();
			const physics::IEnvironment *GetPhysicsEnvironment() const;
			physics::SurfaceMaterial &CreateSurfaceMaterial(const std::string &identifier, Float friction = 0.5f, Float restitution = 0.5f);
			physics::SurfaceMaterial *GetSurfaceMaterial(const std::string &id);
			physics::SurfaceMaterial *GetSurfaceMaterial(UInt32 id);
			std::vector<physics::SurfaceMaterial> *GetSurfaceMaterials();

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
			std::shared_ptr<asset::Model> CreateModel(const std::string &mdl) const;
			std::shared_ptr<asset::Model> CreateModel(bool bAddReference = true) const;
			std::shared_ptr<asset::Model> LoadModel(const std::string &mdl, bool bReload = false);
			bool PrecacheModel(const std::string &mdl);
			bool RunLua(const std::string &lua, const std::string &chunkName);
			LuaDirectoryWatcherManager &GetLuaScriptWatcher();
			util::ResourceWatcherManager &GetResourceWatcher();
			void ReloadGameModeScripts();

			virtual std::shared_ptr<geometry::ModelMesh> CreateModelMesh() const = 0;
			virtual std::shared_ptr<geometry::ModelSubMesh> CreateModelSubMesh() const = 0;

			virtual void GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const = 0;
			void ScheduleEntityForRemoval(ecs::BaseEntity &ent);

			virtual NetEventId FindNetEvent(const std::string &name) const = 0;
			virtual NetEventId SetupNetEvent(const std::string &name) = 0;

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

			const EntityComponentManager &GetEntityComponentManager() const;
			EntityComponentManager &GetEntityComponentManager();

			// Entities
			const std::vector<ecs::BaseEntity *> &GetBaseEntities() const;
			std::vector<ecs::BaseEntity *> &GetBaseEntities();
			std::size_t GetBaseEntityCount() const;
			virtual void GetEntities(std::vector<ecs::BaseEntity *> **ents);
			void GetSpawnedEntities(std::vector<ecs::BaseEntity *> *ents);

			virtual void GetPlayers(std::vector<ecs::BaseEntity *> *ents) = 0;
			virtual void GetNPCs(std::vector<ecs::BaseEntity *> *ents) = 0;
			virtual void GetWeapons(std::vector<ecs::BaseEntity *> *ents) = 0;
			virtual void GetVehicles(std::vector<ecs::BaseEntity *> *ents) = 0;

			virtual void GetPlayers(std::vector<EntityHandle> *ents) = 0;
			virtual void GetNPCs(std::vector<EntityHandle> *ents) = 0;
			virtual void GetWeapons(std::vector<EntityHandle> *ents) = 0;
			virtual void GetVehicles(std::vector<EntityHandle> *ents) = 0;

			virtual ecs::BaseEntity *CreateEntity();
			virtual ecs::BaseEntity *CreateEntity(std::string classname);
			virtual void RemoveEntity(ecs::BaseEntity *ent);
			void RemoveEntities();
			virtual ecs::BaseEntity *GetEntity(unsigned int idx);
			virtual ecs::BaseEntity *GetEntityByLocalIndex(uint32_t idx);
			const ecs::BaseEntity *FindEntityByUniqueId(const util::Uuid &uuid) const { return const_cast<Game *>(this)->FindEntityByUniqueId(uuid); }
			ecs::BaseEntity *FindEntityByUniqueId(const util::Uuid &uuid);
			const std::unordered_map<size_t, ecs::BaseEntity *> &GetEntityUuidMap() const { return const_cast<Game *>(this)->GetEntityUuidMap(); }
			std::unordered_map<size_t, ecs::BaseEntity *> &GetEntityUuidMap() { return m_uuidToEnt; }
			BaseWorldComponent *GetWorld();
			const std::vector<util::TWeakSharedHandle<BaseWorldComponent>> &GetWorldComponents() const;
			unsigned char GetPlayerCount();
			unsigned int GetEntityCount();
			virtual void SpawnEntity(ecs::BaseEntity *ent);
			void SplashDamage(const Vector3 &origin, Float radius, game::DamageInfo &dmg, const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback = nullptr);
			void SplashDamage(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, ecs::BaseEntity *attacker = nullptr, ecs::BaseEntity *inflictor = nullptr, const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback = nullptr);
			void SplashDamage(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, const EntityHandle &attacker = EntityHandle(), const EntityHandle &inflictor = EntityHandle(), const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback = nullptr);

			Bool Overlap(const physics::TraceData &data, std::vector<physics::TraceResult> *optOutResults) const;
			Bool RayCast(const physics::TraceData &data, std::vector<physics::TraceResult> *optOutResults) const;
			Bool Sweep(const physics::TraceData &data, std::vector<physics::TraceResult> *optOutResults) const;

			physics::TraceResult Overlap(const physics::TraceData &data) const;
			physics::TraceResult RayCast(const physics::TraceData &data) const;
			physics::TraceResult Sweep(const physics::TraceData &data) const;

			virtual void CreateGiblet(const GibletCreateInfo &info) = 0;

			const std::shared_ptr<nav::Mesh> &GetNavMesh() const;
			std::shared_ptr<nav::Mesh> &GetNavMesh();

			bool IsMultiPlayer() const;
			bool IsSinglePlayer() const;
			// Map
			bool IsMapInitialized() const;
			bool IsGameReady() const;
			virtual bool LoadMap(const std::string &map, const Vector3 &origin = {}, std::vector<EntityHandle> *entities = nullptr);
			// Called when map and gamemode has been fully loaded and the game can start proper
			virtual void OnGameReady();
			bool LoadNavMesh(bool bReload = false);
			game::AmmoTypeManager &GetAmmoTypeManager();
			Bool RegisterAmmoType(const std::string &name, Int32 damage = 10, Float force = 200.f, DamageType dmgType = Bullet, game::AmmoType **ammoOut = nullptr);
			game::AmmoType *GetAmmoType(const std::string &name, UInt32 *ammoId = nullptr);
			game::AmmoType *GetAmmoType(UInt32 ammoId);
			const game::GameModeInfo *GetGameMode() const;
			game::GameModeInfo *GetGameMode();
			void SetGameMode(const std::string &gameMode);
			ecs::BaseEntity *GetGameModeEntity();
			ecs::BaseEntity *GetGameEntity();
			LuaEntityManager &GetLuaEntityManager();
			const LuaEntityManager &GetLuaEntityManager() const { return const_cast<Game *>(this)->GetLuaEntityManager(); }
			AnimationUpdateManager &GetAnimationUpdateManager();
			void UpdatePackagePaths();

			void SetWorld(BaseWorldComponent *entWorld);
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
			bool CollisionTest(BasePlayerComponent &pl, float *distance, Vector3 *hitnormal = nullptr);
			void CollisionTest(ecs::BaseEntity *a, ecs::BaseEntity *b);
			void EnableCollisions(bool b);
			//
		  public:
			//
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
			lua::State *GetLuaState();
			Lua::Interface &GetLuaInterface();
			virtual void RegisterLua();
			virtual void RegisterLuaGlobals();
			virtual void RegisterLuaClasses();
			void RegisterLuaGameClasses(luabind::module_ &gameMod);
			virtual void RegisterLuaLibraries();
			virtual bool RegisterNetMessage(std::string name);
			void RegisterLuaNetMessage(std::string name, int handler);
			std::vector<std::string> *GetLuaNetMessageIndices();
			bool BroadcastEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx);
			bool InjectEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx);
			Lua::StatusCode LoadLuaFile(std::string &fInOut, fs::SearchFlags includeFlags = fs::SearchFlags::All, fs::SearchFlags excludeFlags = fs::SearchFlags::None);
			virtual bool ExecuteLuaFile(std::string &fInOut, lua::State *optCustomLuaState = nullptr, bool addToIncludeCache = false);
			// Same as ExecuteLuaFile, but uses the last value from the include stack
			//bool IncludeLuaFile(std::string &fInOut); // Deprecated

			virtual bool RunLua(const std::string &lua) = 0;
			virtual void RunLuaFiles(const std::string &subPath);
			Lua::StatusCode ProtectedLuaCall(const std::function<Lua::StatusCode(lua::State *)> &pushFuncArgs, int32_t numResults);
			template<class TLuaEntity, class THandle>
			ecs::BaseEntity *CreateLuaEntity(std::string classname, luabind::object &oClass, bool bLoadIfNotExists = false);
			virtual ecs::BaseEntity *CreateLuaEntity(std::string classname, bool bLoadIfNotExists = false) = 0;
			virtual BaseEntityComponent *CreateLuaEntityComponent(ecs::BaseEntity &ent, std::string classname) = 0;
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
			const LuaCore::ClassManager &GetLuaClassManager() const;
			LuaCore::ClassManager &GetLuaClassManager();

			CallbackHandle AddConVarCallback(const std::string &cvar, LuaFunction function);
			unsigned int GetNetMessageID(std::string name);
			std::string *GetNetMessageIdentifier(unsigned int ID);
			virtual void OnPlayerDropped(BasePlayerComponent &pl, networking::DropReason reason);
			virtual void OnPlayerReady(BasePlayerComponent &pl);
			virtual void OnPlayerJoined(BasePlayerComponent &pl);
			// Timers
			Timer *CreateTimer(float delay, int reps, LuaFunctionObject luaFunction, TimerType timeType = TimerType::CurTime);
			Timer *CreateTimer(float delay, int reps, const CallbackHandle &hCallback, TimerType timeType = TimerType::CurTime);
			void ClearTimers();
			// ConVars
			template<class T>
			T *GetConVar(const std::string &scmd);
			console::ConConf *GetConVar(const std::string &scmd);
			int GetConVarInt(const std::string &scmd);
			std::string GetConVarString(const std::string &scmd);
			float GetConVarFloat(const std::string &scmd);
			bool GetConVarBool(const std::string &scmd);
			console::ConVarFlags GetConVarFlags(const std::string &scmd);
			const std::unordered_map<std::string, std::vector<console::CvarCallback>> &GetConVarCallbacks() const;

			virtual Float GetFrictionScale() const = 0;
			virtual Float GetRestitutionScale() const = 0;
			const MapInfo &GetMapInfo() const;

			void SetGameFlags(GameFlags flags);
			GameFlags GetGameFlags() const;

			virtual bool IsPhysicsSimulationEnabled() const = 0;

			std::vector<ComponentHandle<BasePhysicsComponent>> &GetAwakePhysicsComponents();
			std::vector<BaseEntityComponent *> &GetEntityTickComponents() { return m_entityTickComponents; }
			std::vector<BaseGamemodeComponent *> &GetGamemodeComponents() { return m_gamemodeComponents; }

			// Debug
			virtual void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration = 0.f) = 0;
			virtual void DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration = 0.f) = 0;
			virtual void DrawPlane(const Vector3 &n, float dist, const Color &color, float duration = 0.f) = 0;
			virtual void DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration = 0.f) = 0;
			debug::ProfilingStageManager<debug::ProfilingStage> *GetProfilingStageManager();
			bool StartProfilingStage(const char *stage);
			bool StopProfilingStage();
		  protected:
			virtual void UpdateTime();
			void GetLuaRegisteredEntities(std::vector<std::string> &luaClasses) const;

			GameFlags m_flags = GameFlags::InitialTick;
			std::unique_ptr<AnimationUpdateManager> m_animUpdateManager;
			std::vector<ecs::BaseEntity *> m_baseEnts;
			std::unordered_map<size_t, ecs::BaseEntity *> m_uuidToEnt;
			std::queue<EntityHandle> m_entsScheduledForRemoval;
			std::vector<ComponentHandle<BasePhysicsComponent>> m_awakePhysicsEntities;
			std::vector<BaseEntityComponent *> m_entityTickComponents;
			std::vector<BaseGamemodeComponent *> m_gamemodeComponents;
			std::shared_ptr<Lua::Interface> m_lua = nullptr;
			std::unique_ptr<LuaCore::ClassManager> m_luaClassManager;
			std::unique_ptr<LuaDirectoryWatcherManager> m_scriptWatcher = nullptr;
			std::unique_ptr<physics::SurfaceMaterialManager> m_surfaceMaterialManager = nullptr;
			std::unordered_map<std::string, std::vector<console::CvarCallback>> m_cvarCallbacks;
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
			std::vector<util::TWeakSharedHandle<BaseWorldComponent>> m_worldComponents {};
			game::GameModeInfo *m_gameMode = nullptr;
			EntityHandle m_entGamemode;
			EntityHandle m_entGame;
			CallbackHandle m_cbProfilingHandle = {};
			std::unique_ptr<debug::ProfilingStageManager<debug::ProfilingStage>> m_profilingStageManager;
			std::shared_ptr<nav::Mesh> m_navMesh = nullptr;
			std::unique_ptr<game::AmmoTypeManager> m_ammoTypes = nullptr;
			std::unique_ptr<LuaEntityManager> m_luaEnts = nullptr;
			std::shared_ptr<EntityComponentManager> m_componentManager = nullptr;

			// Lua
			std::vector<std::string> m_luaIncludeStack = {};

			virtual void InitializeWorldData(asset::WorldData &worldData);
			virtual void InitializeMapEntities(asset::WorldData &worldData, std::vector<EntityHandle> &outEnts);
			virtual bool LoadLuaComponent(const std::string &luaFilePath, const std::string &mainPath, const std::string &componentName);
			virtual bool InitializeGameMode();
			template<class TComponent, class THolder>
			BaseEntityComponent *CreateLuaEntityComponent(ecs::BaseEntity &ent, std::string classname);

			virtual bool InvokeEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject);
			virtual void RegisterLuaEntityComponents(luabind::module_ &gameMod);
			void LoadConfig();
			void SaveConfig();
			void UpdateTimers();
			virtual void InitializeLuaScriptWatcher();

			// Map
			ecs::BaseEntity *CreateMapEntity(asset::EntityData &entData);
			BaseEntityComponent *CreateMapComponent(ecs::BaseEntity &ent, const std::string &componentType, const asset::ComponentData &componentData);
			std::unique_ptr<physics::IEnvironment, void (*)(physics::IEnvironment *)> m_physEnvironment = std::unique_ptr<physics::IEnvironment, void (*)(physics::IEnvironment *)> {nullptr, [](physics::IEnvironment *) {}};

			virtual std::shared_ptr<EntityComponentManager> InitializeEntityComponentManager() = 0;
			virtual void OnEntityCreated(ecs::BaseEntity *ent);
			virtual void SetupEntity(ecs::BaseEntity *ent, unsigned int idx) = 0;
			virtual unsigned int GetFreeEntityIndex() = 0;
			virtual std::shared_ptr<nav::Mesh> LoadNavMesh(const std::string &fname);
			void SetupEntity(ecs::BaseEntity *ent);
			virtual void InitializeEntityComponents(EntityComponentManager &componentManager);
			virtual void OnMapLoaded();
		};
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::Game::GameFlags)

	DLLNETWORK void IncludeLuaEntityBaseClasses(lua::State *l, int refEntities, int obj, int data);
	namespace pragma {
		template<class T>
		T *Game::GetConVar(const std::string &scmd)
		{
			console::ConConf *cv = GetConVar(scmd);
			if(cv == nullptr)
				return nullptr;
			return static_cast<T *>(cv);
		}

		template<class TLuaEntity, class THandle>
		ecs::BaseEntity *Game::CreateLuaEntity(std::string classname, luabind::object &oClass, bool bLoadIfNotExists)
		{
			auto *o = m_luaEnts->GetClassObject(classname);
			if(o == nullptr) {
				if(bLoadIfNotExists && LoadLuaEntityByClass(classname) == true)
					return CreateLuaEntity<TLuaEntity, THandle>(classname, oClass, false);
				return nullptr;
			}
			luabind::object r;
			ecs::BaseEntity *el = nullptr;
#ifndef LUABIND_NO_EXCEPTIONS
			try {
#endif
				r = (*o)();
				auto *elLua = luabind::object_cast<TLuaEntity *>(r);
				auto *holder = luabind::object_cast<THandle *>(r);
				if(elLua && holder) {
					elLua->SetupLua(r, classname);
					holder->SetHandle(util::weak_shared_handle_cast<ecs::BaseEntity, TLuaEntity>(elLua->GetHandle()));
					el = elLua;
				}
				else {
					Con::CSV << "WARNING: Unable to create lua entity '" << classname << "': Lua class is not derived from valid entity base!" << Con::endl;
					return nullptr;
				}
#ifndef LUABIND_NO_EXCEPTIONS
			}
			catch(luabind::error &) {
				Lua::HandleLuaError(GetLuaState());
				return nullptr;
			}
#endif
			if(!el) {
				Con::CWAR << "Unable to create lua entity '" << classname << "'!" << Con::endl;
				return nullptr;
			}
			oClass = *o;
			return el;
		}

		template<class TComponent, class THolder>
		BaseEntityComponent *Game::CreateLuaEntityComponent(ecs::BaseEntity &ent, std::string classname)
		{
			auto *o = m_luaEnts->GetComponentClassObject(classname);
			if(o == nullptr)
				return nullptr;
			luabind::object r;
			BaseEntityComponent *el = nullptr;
#ifndef LUABIND_NO_EXCEPTIONS
			try {
#endif
				r = (*o)(ent.GetLuaObject());

				auto *elLua = luabind::object_cast<TComponent *>(r);
				auto *holder = luabind::object_cast<THolder *>(r);
				if(elLua && holder) {
					elLua->SetupLua(r);
					holder->SetHandle(util::weak_shared_handle_cast<BaseEntityComponent, TComponent>(elLua->GetHandle()));
					el = elLua;
				}
				else {
					Con::CSV << "WARNING: Unable to create lua entity component '" << classname << "': Lua class is not derived from valid entity component base!" << Con::endl;
					return nullptr;
				}
#ifndef LUABIND_NO_EXCEPTIONS
			}
			catch(luabind::error &) {
				Lua::HandleLuaError(GetLuaState());
				return nullptr;
			}
#endif
			if(!r) {
				Con::CWAR << "Unable to create lua entity component '" << classname << "'!" << Con::endl;
				return nullptr;
			}
			return el;
		}
	}

	DLLNETWORK bool NetIncludePacketID(pragma::NetworkState *state, std::string identifier, NetPacket &packet, NetPacket &packetNew);
};
