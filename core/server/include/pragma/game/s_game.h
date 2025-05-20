/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_GAME_H__
#define __S_GAME_H__
#include <pragma/game/game.h>
#include "pragma/serverdefinitions.h"
#include "pragma/entities/world.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <mathutil/color.h>
#include <sharedutils/datastream.h>
#ifdef __linux__
#include "pragma/cacheinfo.h"
#endif

struct CacheInfo;
class SBaseEntity;
namespace pragma {
	class SPlayerComponent;
	namespace ai {
		class TaskManager;
	};
	namespace networking {
		class IServerClient;
		class ClientRecipientFilter;
	};
};
namespace udm {
	struct Property;
	using PProperty = std::shared_ptr<Property>;
};
enum class CLIENT_DROPPED;
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
	mutable std::unique_ptr<pragma::ai::TaskManager> m_taskManager;
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

	virtual bool InvokeEntityEvent(pragma::BaseEntityComponent &component, uint32_t eventId, int32_t argsIdx, bool bInject) override;
	virtual void OnEntityCreated(BaseEntity *ent) override;
	virtual unsigned int GetFreeEntityIndex() override;
	virtual void SetupEntity(BaseEntity *ent, unsigned int idx) override;
	virtual void InitializeLuaScriptWatcher() override;
	virtual std::shared_ptr<pragma::EntityComponentManager> InitializeEntityComponentManager() override;
	virtual void InitializeEntityComponents(pragma::EntityComponentManager &componentManager) override;
	virtual void RegisterLuaEntityComponents(luabind::module_ &gameMod) override;
	virtual void RegisterLuaEntityComponent(luabind::class_<pragma::BaseEntityComponent> &classDef) override;
	virtual bool InitializeGameMode() override;

	const pragma::NetEventManager &GetEntityNetEventManager() const;
	pragma::NetEventManager &GetEntityNetEventManager();

	virtual std::string GetLuaNetworkDirectoryName() const override;
	virtual std::string GetLuaNetworkFileName() const override;
	virtual bool LoadLuaComponent(const std::string &luaFilePath, const std::string &mainPath, const std::string &componentName) override;

	// Resources which can be requested by clients, if they don't have them
	std::vector<std::string> m_gameResources;

	pragma::NetEventManager m_entNetEventManager = {};
	CallbackHandle m_cbProfilingHandle = {};
	std::unique_ptr<pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage>> m_profilingStageManager;
  public:
	using Game::LoadLuaComponent;
	virtual void InitializeLua() override;
	virtual void SetupLua() override;
	virtual void SetUp() override;
	virtual void Think() override;
	virtual void Tick() override;
	virtual void Initialize() override;
	SGame(NetworkState *state);
	virtual ~SGame() override;
	virtual void OnRemove() override;
	virtual bool IsServer() override;
	virtual bool IsClient() override;
	virtual void RegisterLua() override;
	virtual void RegisterLuaLibraries() override;
	virtual void RegisterLuaClasses() override;
	void SendSnapshot();
	void SendSnapshot(pragma::SPlayerComponent *pl);
	virtual std::shared_ptr<ModelMesh> CreateModelMesh() const override;
	virtual std::shared_ptr<ModelSubMesh> CreateModelSubMesh() const override;
	virtual void GetRegisteredEntities(std::vector<std::string> &classes, std::vector<std::string> &luaClasses) const override;
	virtual bool RunLua(const std::string &lua) override;
	void RegisterGameResource(const std::string &fileName);
	bool IsValidGameResource(const std::string &fileName);
	virtual void CreateGiblet(const GibletCreateInfo &info) override;
	virtual pragma::BaseEntityComponent *CreateLuaEntityComponent(BaseEntity &ent, std::string classname) override;

	std::vector<std::string> &GetNetEventIds();
	const std::vector<std::string> &GetNetEventIds() const;

	pragma::debug::ProfilingStageManager<pragma::debug::ProfilingStage> *GetProfilingStageManager();
	bool StartProfilingStage(const char *stage);
	bool StopProfilingStage();

	void ChangeLevel(const std::string &mapName, const std::string &landmarkName = "");

	pragma::NetEventId RegisterNetEvent(const std::string &name);
	virtual pragma::NetEventId FindNetEvent(const std::string &name) const override;
	virtual pragma::NetEventId SetupNetEvent(const std::string &name) override;

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
	virtual void RemoveEntity(BaseEntity *ent) override;
	pragma::SPlayerComponent *GetPlayer(pragma::networking::IServerClient &session);
	virtual void SpawnEntity(BaseEntity *ent) override;
	void GetEntities(std::vector<SBaseEntity *> **ents);
	void GetPlayers(std::vector<BaseEntity *> *ents) override;
	void GetNPCs(std::vector<BaseEntity *> *ents) override;
	void GetWeapons(std::vector<BaseEntity *> *ents) override;
	void GetVehicles(std::vector<BaseEntity *> *ents) override;

	void GetPlayers(std::vector<SBaseEntity *> *ents);
	void GetNPCs(std::vector<SBaseEntity *> *ents);
	void GetWeapons(std::vector<SBaseEntity *> *ents);
	void GetVehicles(std::vector<SBaseEntity *> *ents);

	void GetPlayers(std::vector<EntityHandle> *ents) override;
	void GetNPCs(std::vector<EntityHandle> *ents) override;
	void GetWeapons(std::vector<EntityHandle> *ents) override;
	void GetVehicles(std::vector<EntityHandle> *ents) override;

	bool RegisterNetMessage(std::string name) override;
	void HandleLuaNetPacket(pragma::networking::IServerClient &session, NetPacket &packet);

	void ReceiveUserInfo(pragma::networking::IServerClient &session, NetPacket &packet);
	void ReceiveGameReady(pragma::networking::IServerClient &session, NetPacket &packet);
	void OnClientConVarChanged(pragma::BasePlayerComponent &pl, std::string cvar, std::string value);
	void OnClientDropped(pragma::networking::IServerClient &client, pragma::networking::DropReason reason);

	virtual Float GetFrictionScale() const override;
	virtual Float GetRestitutionScale() const override;

	pragma::ai::TaskManager &GetAITaskManager() const;

	virtual bool IsPhysicsSimulationEnabled() const override;

	// Map
	virtual bool LoadMap(const std::string &map, const Vector3 &origin = {}, std::vector<EntityHandle> *entities = nullptr) override;

	// Debug
	virtual void DrawLine(const Vector3 &start, const Vector3 &end, const Color &color, float duration = 0.f) override;
	virtual void DrawBox(const Vector3 &origin, const Vector3 &start, const Vector3 &end, const EulerAngles &ang, const Color &colorOutline, const std::optional<Color> &fillColor, float duration = 0.f) override;
	virtual void DrawPlane(const Vector3 &n, float dist, const Color &color, float duration = 0.f) override;
	virtual void DrawMesh(const std::vector<Vector3> &meshVerts, const Color &color, const Color &colorOutline, float duration = 0.f) override;

	void SpawnPlayer(pragma::BasePlayerComponent &pl);

	void CreateExplosion(const Vector3 &origin, Float radius, DamageInfo &dmg, const std::function<bool(BaseEntity *, DamageInfo &)> &callback = nullptr);
	void CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, BaseEntity *attacker = nullptr, BaseEntity *inflictor = nullptr, const std::function<bool(BaseEntity *, DamageInfo &)> &callback = nullptr);
	void CreateExplosion(const Vector3 &origin, Float radius, UInt32 damage, Float force = 0.f, const EntityHandle &attacker = EntityHandle(), const EntityHandle &inflictor = EntityHandle(), const std::function<bool(BaseEntity *, DamageInfo &)> &callback = nullptr);

	void WriteEntityData(NetPacket &packet, SBaseEntity **ents, uint32_t entCount, pragma::networking::ClientRecipientFilter &rp);
};
#pragma warning(pop)

template<class T>
T *SGame::CreateEntity(unsigned int idx)
{
	if(umath::is_flag_set(m_flags, GameFlags::ClosingGame))
		return nullptr;
	T *ent = new T();
	SetupEntity(ent, idx);
	return ent;
}

template<class T>
T *SGame::CreateEntity()
{
	if(umath::is_flag_set(m_flags, GameFlags::ClosingGame))
		return nullptr;
	return CreateEntity<T>(GetFreeEntityIndex());
}

#endif
