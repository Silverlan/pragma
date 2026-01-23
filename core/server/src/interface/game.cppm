// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:game;

export import :ai.schedule;
export import :core.cache_info;
export import :entities;
export import :entities.components;
export import pragma.shared;

export namespace pragma {
#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLSERVER SGame : public Game {
	  private:
		std::vector<SBaseEntity *> m_ents;
		std::unique_ptr<CacheInfo> m_luaCache = nullptr;
		struct ChangeLevelInfo {
			std::string map;
			std::string landmarkName;
		};
		std::optional<ChangeLevelInfo> m_changeLevelInfo = {};
		mutable std::unique_ptr<ai::TaskManager> m_taskManager;
		// The state of the world before the level transition (if there was one). Each key is a global entity name, and the value is the data stream object for that entity.
		std::unordered_map<std::string, udm::PProperty> m_preTransitionWorldState {};
		// Delta landmark offset between this level and the previous level (in case there was a level change)
		Vector3 m_deltaTransitionLandmarkOffset {};
	  protected:
		template<class T>
		void GetPlayers(std::vector<T *> *ents);
		template<class T>
		void GetNPCs(std::vector<T *> *ents);
		template<class T>
		void GetWeapons(std::vector<T *> *ents);
		template<class T>
		void GetVehicles(std::vector<T *> *ents);

		virtual bool InvokeEntityEvent(BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject) override;
		virtual void OnEntityCreated(ecs::BaseEntity *ent) override;
		virtual unsigned int GetFreeEntityIndex() override;
		virtual void SetupEntity(ecs::BaseEntity *ent, unsigned int idx) override;
		virtual void InitializeLuaScriptWatcher() override;
		virtual std::shared_ptr<EntityComponentManager> InitializeEntityComponentManager() override;
		virtual void InitializeEntityComponents(EntityComponentManager &componentManager) override;
		virtual void RegisterLuaEntityComponents(luabind::module_ &gameMod) override;
		virtual bool InitializeGameMode() override;

		const NetEventManager &GetEntityNetEventManager() const;
		NetEventManager &GetEntityNetEventManager();

		virtual std::string GetLuaNetworkDirectoryName() const override;
		virtual std::string GetLuaNetworkFileName() const override;
		virtual bool LoadLuaComponent(const std::string &luaFilePath, const std::string &mainPath, const std::string &componentName) override;

		// Resources which can be requested by clients, if they don't have them
		std::vector<std::string> m_gameResources;

		NetEventManager m_entNetEventManager = {};
		CallbackHandle m_cbProfilingHandle = {};
		std::unique_ptr<debug::ProfilingStageManager<debug::ProfilingStage>> m_profilingStageManager;
	  public:
		using Game::LoadLuaComponent;
		virtual void InitializeLua() override;
		virtual void SetupLua() override;
		virtual void SetUp() override;
		virtual void Think() override;
		virtual void Tick() override;
		virtual void Initialize() override;
		static SGame *Get();
		SGame(NetworkState *state);
		virtual ~SGame() override;
		virtual void OnRemove() override;
		virtual bool IsServer() override;
		virtual bool IsClient() override;
		virtual void RegisterLua() override;
		virtual void RegisterLuaLibraries() override;
		virtual void RegisterLuaClasses() override;
		void SendSnapshot();
		void SendSnapshot(SPlayerComponent *pl);
		virtual std::shared_ptr<geometry::ModelMesh> CreateModelMesh() const override;
		virtual std::shared_ptr<geometry::ModelSubMesh> CreateModelSubMesh() const override;
		virtual void GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const override;
		virtual bool RunLua(const std::string &lua) override;
		void RegisterGameResource(const std::string &fileName);
		bool IsValidGameResource(const std::string &fileName);
		virtual void CreateGiblet(const GibletCreateInfo &info) override;
		virtual BaseEntityComponent *CreateLuaEntityComponent(ecs::BaseEntity &ent, std::string classname) override;

		std::vector<std::string> &GetNetEventIds();
		const std::vector<std::string> &GetNetEventIds() const;

		debug::ProfilingStageManager<debug::ProfilingStage> *GetProfilingStageManager();
		bool StartProfilingStage(const char *stage);
		bool StopProfilingStage();

		void ChangeLevel(const std::string &mapName, const std::string &landmarkName = "");

		NetEventId RegisterNetEvent(const std::string &name);
		virtual NetEventId FindNetEvent(const std::string &name) const override;
		virtual NetEventId SetupNetEvent(const std::string &name) override;

		virtual float GetTimeScale() override;
		virtual void SetTimeScale(float t) override;
		// Lua
		void GenerateLuaCache();
		CacheInfo *GetLuaCacheInfo();
		void UpdateLuaCache(const std::string &f);

		// Entities
		virtual SBaseEntity *CreateLuaEntity(std::string classname, bool bLoadIfNotExists = false) override;
		virtual SBaseEntity *GetEntity(unsigned int idx) override;
		virtual SBaseEntity *CreateEntity(std::string classname) override;
		template<class T>
		T *CreateEntity();
		template<class T>
		T *CreateEntity(unsigned int idx);
		virtual void RemoveEntity(ecs::BaseEntity *ent) override;
		SPlayerComponent *GetPlayer(networking::IServerClient &session);
		virtual void SpawnEntity(ecs::BaseEntity *ent) override;
		void GetEntities(std::vector<SBaseEntity *> **ents);
		void GetPlayers(std::vector<ecs::BaseEntity *> *ents) override;
		void GetNPCs(std::vector<ecs::BaseEntity *> *ents) override;
		void GetWeapons(std::vector<ecs::BaseEntity *> *ents) override;
		void GetVehicles(std::vector<ecs::BaseEntity *> *ents) override;

		void GetPlayers(std::vector<SBaseEntity *> *ents);
		void GetNPCs(std::vector<SBaseEntity *> *ents);
		void GetWeapons(std::vector<SBaseEntity *> *ents);
		void GetVehicles(std::vector<SBaseEntity *> *ents);

		void GetPlayers(std::vector<EntityHandle> *ents) override;
		void GetNPCs(std::vector<EntityHandle> *ents) override;
		void GetWeapons(std::vector<EntityHandle> *ents) override;
		void GetVehicles(std::vector<EntityHandle> *ents) override;

		bool RegisterNetMessage(std::string name) override;
		void HandleLuaNetPacket(networking::IServerClient &session, NetPacket &packet);

		void ReceiveUserInfo(networking::IServerClient &session, NetPacket &packet);
		void ReceiveGameReady(networking::IServerClient &session, NetPacket &packet);
		void OnClientConVarChanged(BasePlayerComponent &pl, std::string cvar, std::string value);
		void OnClientDropped(networking::IServerClient &client, networking::DropReason reason);

		virtual Float GetFrictionScale() const override;
		virtual Float GetRestitutionScale() const override;

		ai::TaskManager &GetAITaskManager() const;

		virtual bool IsPhysicsSimulationEnabled() const override;

		// Map
		virtual bool LoadMap(const std::string &map, const Vector3 &origin = {}, std::vector<EntityHandle> *entities = nullptr) override;

		// Debug
		virtual void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration = 0.f) override;
		virtual void DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration = 0.f) override;
		virtual void DrawPlane(const Vector3 &n, float dist, const Color &color, float duration = 0.f) override;
		virtual void DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration = 0.f) override;

		void SpawnPlayer(BasePlayerComponent &pl);

		void CreateExplosion(const Vector3 &origin, Float radius, game::DamageInfo &dmg, const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback = nullptr);
		void CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, ecs::BaseEntity *attacker = nullptr, ecs::BaseEntity *inflictor = nullptr, const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback = nullptr);
		void CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, const EntityHandle &attacker = EntityHandle(), const EntityHandle &inflictor = EntityHandle(), const std::function<bool(ecs::BaseEntity *, game::DamageInfo &)> &callback = nullptr);

		void WriteEntityData(NetPacket &packet, SBaseEntity **ents, uint32_t entCount, networking::ClientRecipientFilter &rp);
	};
#pragma warning(pop)

	template<class T>
	T *SGame::CreateEntity(unsigned int idx)
	{
		if(math::is_flag_set(m_flags, GameFlags::ClosingGame))
			return nullptr;
		T *ent = new T();
		SetupEntity(ent, idx);
		return ent;
	}

	template<class T>
	T *SGame::CreateEntity()
	{
		if(math::is_flag_set(m_flags, GameFlags::ClosingGame))
			return nullptr;
		return CreateEntity<T>(GetFreeEntityIndex());
	}

	template<class T>
	void SGame::GetPlayers(std::vector<T *> *ents)
	{
		auto &players = SPlayerComponent::GetAll();
		ents->reserve(ents->size() + players.size());
		for(auto *pl : players)
			ents->push_back(static_cast<T *>(&pl->GetEntity()));
	}

	template<class T>
	void SGame::GetNPCs(std::vector<T *> *ents)
	{
		auto &npcs = SPlayerComponent::GetAll();
		ents->reserve(ents->size() + npcs.size());
		for(auto *npc : npcs)
			ents->push_back(static_cast<T *>(&npc->GetEntity()));
	}

	template<class T>
	void SGame::GetWeapons(std::vector<T *> *ents)
	{
		auto &weapons = SPlayerComponent::GetAll();
		ents->reserve(ents->size() + weapons.size());
		for(auto *wep : weapons)
			ents->push_back(static_cast<T *>(&wep->GetEntity()));
	}

	template<class T>
	void SGame::GetVehicles(std::vector<T *> *ents)
	{
		auto &vehicles = SPlayerComponent::GetAll();
		ents->reserve(ents->size() + vehicles.size());
		for(auto *vhc : vehicles)
			ents->push_back(static_cast<T *>(&vhc->GetEntity()));
	}
}
