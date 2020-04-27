/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASEENTITY_H__
#define __BASEENTITY_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/baseluaobj.h"
#include "pragma/entities/entity_component_system.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include <pragma/console/conout.h>

#define DECLARE_ENTITY_HANDLE \
	protected: \
	virtual void InitializeHandle() override; \
	public: \
	virtual void InitializeLuaObject(lua_State *lua) override; \

#define DEFINE_ENTITY_HANDLE(localname,handleName) \
	void localname::InitializeLuaObject(lua_State *lua) {BaseEntity::InitializeLuaObject<handleName>(lua);} \
	void localname::InitializeHandle() {return BaseEntity::InitializeHandle<handleName>();}

class Engine;
class NetworkState;
class DamageInfo;
class TraceData;
class Model;
class ALSound;
class PhysObj;

enum class ALSoundType : Int32;
enum class Activity : uint16_t;
enum class CollisionMask : uint32_t;
enum class PHYSICSTYPE : int;

namespace pragma
{
	class BaseEntityComponent;
	class BaseModelComponent;
	class BaseAnimatedComponent;
	class BaseWeaponComponent;
	class BaseVehicleComponent;
	class BaseAIComponent;
	class BaseCharacterComponent;
	class BasePlayerComponent;
	class BasePhysicsComponent;
	class BaseTimeScaleComponent;
	class BaseNameComponent;
	class BaseTransformComponent;
	class BaseParentComponent;
	namespace physics {class Transform; class ScaledTransform;};

	using NetEventId = uint32_t;
};

const double ENT_EPSILON = 0.000'01;

class EntityHandle;
class DataStream;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLNETWORK BaseEntity
	: public LuaObj<EntityHandle>,
	public pragma::BaseEntityComponentSystem
{
public:
	static pragma::ComponentEventId EVENT_HANDLE_KEY_VALUE;
	static pragma::ComponentEventId EVENT_ON_SPAWN;
	static pragma::ComponentEventId EVENT_ON_POST_SPAWN;
	static pragma::ComponentEventId EVENT_ON_REMOVE;

	enum class StateFlags : uint8_t
	{
		None = 0u,
		Spawned = 1u,
		SnapshotUpdateRequired = Spawned<<1u,

		PositionChanged = SnapshotUpdateRequired<<1u,
		RotationChanged = PositionChanged<<1u,
		CollisionBoundsChanged = RotationChanged<<1u,
		RenderBoundsChanged = CollisionBoundsChanged<<1u
	};

	static void RegisterEvents(pragma::EntityComponentManager &componentManager);

	virtual std::string GetClass() const;
	BaseEntity();
	void Construct(unsigned int idx);

	friend EntityHandle;
	friend Engine;
public:
	StateFlags GetStateFlags() const;
	bool HasStateFlag(StateFlags flag) const;
	void SetStateFlag(StateFlags flag);
	void ResetStateChangeFlags();

	virtual bool IsStatic() const;
	bool IsDynamic() const;
	virtual NetworkState *GetNetworkState() const=0;

	pragma::NetEventId FindNetEvent(const std::string &name) const;
	pragma::ComponentEventId RegisterComponentEvent(const std::string &name) const;
	pragma::ComponentEventId GetEventId(const std::string &name) const;

	// Returns ORIGIN if the entity has no transform component
	void GetPose(pragma::physics::ScaledTransform &outTransform) const;
	void GetPose(pragma::physics::Transform &outTransform) const;
	void SetPose(const pragma::physics::ScaledTransform &outTransform);
	void SetPose(const pragma::physics::Transform &outTransform);
	const Vector3 &GetPosition() const;
	void SetPosition(const Vector3 &pos);
	Vector3 GetCenter() const;

	// Returns unit quaternion if entity has no transform component
	const Quat &GetRotation() const;
	void SetRotation(const Quat &rot);
	
	const Vector3 &GetScale() const;
	void SetScale(const Vector3 &scale);

	// Helper functions
	virtual util::WeakHandle<pragma::BaseModelComponent> GetModelComponent() const=0;
	virtual util::WeakHandle<pragma::BaseAnimatedComponent> GetAnimatedComponent() const=0;
	virtual util::WeakHandle<pragma::BaseWeaponComponent> GetWeaponComponent() const=0;
	virtual util::WeakHandle<pragma::BaseVehicleComponent> GetVehicleComponent() const=0;
	virtual util::WeakHandle<pragma::BaseAIComponent> GetAIComponent() const=0;
	virtual util::WeakHandle<pragma::BaseCharacterComponent> GetCharacterComponent() const=0;
	virtual util::WeakHandle<pragma::BasePlayerComponent> GetPlayerComponent() const=0;
	virtual util::WeakHandle<pragma::BasePhysicsComponent> GetPhysicsComponent() const=0;
	virtual util::WeakHandle<pragma::BaseTimeScaleComponent> GetTimeScaleComponent() const=0;
	virtual util::WeakHandle<pragma::BaseNameComponent> GetNameComponent() const=0;
	util::WeakHandle<pragma::BaseTransformComponent> GetTransformComponent() const;

	// These are quick-access functions for commonly used component functions.
	// In some cases these may create the component, if it doesn't exist, and transmit
	// it to the client if called serverside!
	std::shared_ptr<ALSound> CreateSound(const std::string &snd,ALSoundType type);
	std::shared_ptr<ALSound> EmitSound(const std::string &snd,ALSoundType type,float gain=1.f,float pitch=1.f);

	std::string GetName() const;
	void SetName(const std::string &name);

	void SetModel(const std::string &mdl);
	void SetModel(const std::shared_ptr<Model> &mdl);
	std::shared_ptr<Model> GetModel() const;
	std::string GetModelName() const;
	std::optional<pragma::physics::Transform> GetAttachmentPose(uint32_t attId) const;
	uint32_t GetSkin() const;
	void SetSkin(uint32_t skin);
	uint32_t GetBodyGroup(const std::string &name) const;
	void SetBodyGroup(const std::string &name,uint32_t id);

	void SetParent(BaseEntity *parent);
	void ClearParent();
	pragma::BaseParentComponent *GetParent() const;

	PhysObj *GetPhysicsObject() const;
	PhysObj *InitializePhysics(PHYSICSTYPE type);
	void DestroyPhysicsObject();
	void DropToFloor();
	std::pair<Vector3,Vector3> GetCollisionBounds() const;
	void SetCollisionFilterMask(CollisionMask filterMask);
	void SetCollisionFilterGroup(CollisionMask filterGroup);
	CollisionMask GetCollisionFilterGroup() const;
	CollisionMask GetCollisionFilterMask() const;

	Vector3 GetForward() const;
	Vector3 GetUp() const;
	Vector3 GetRight() const;

	void Input(const std::string &input,BaseEntity *activator=nullptr,BaseEntity *caller=nullptr,const std::string &data="");

	uint16_t GetHealth() const;
	uint16_t GetMaxHealth() const;
	void SetHealth(uint16_t health);
	void SetMaxHealth(uint16_t maxHealth);

	void SetVelocity(const Vector3 &vel);
	void AddVelocity(const Vector3 &vel);
	Vector3 GetVelocity() const;
	void SetAngularVelocity(const Vector3 &vel);
	void AddAngularVelocity(const Vector3 &vel);
	Vector3 GetAngularVelocity() const;

	void PlayAnimation(int32_t animation,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
	void PlayLayeredAnimation(int32_t slot,int32_t animation,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
	bool PlayActivity(Activity activity,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
	bool PlayLayeredActivity(int32_t slot,Activity activity,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
	bool PlayLayeredAnimation(int32_t slot,std::string animation,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
	void StopLayeredAnimation(int slot);
	bool PlayAnimation(const std::string &animation,pragma::FPlayAnim flags=pragma::FPlayAnim::Default);
	int32_t GetAnimation() const;
	Activity GetActivity() const;

	void TakeDamage(DamageInfo &info);
	//

	// Returns true if this entity is local to the current network state (i.e. clientside/serverside only)
	virtual bool IsNetworkLocal() const=0;

	void SetSpawnFlags(uint32_t spawnFlags);
	unsigned int GetSpawnFlags() const;

	void RemoveEntityOnRemoval(BaseEntity *ent,Bool bRemove=true);
	void RemoveEntityOnRemoval(const EntityHandle &hEnt,Bool bRemove=true);

	CallbackHandle CallOnRemove(const CallbackHandle &hCallback);

	bool IsMapEntity() const;

	virtual void OnRemove();
	// Returns true if the entity is static, or asleep (and not animated)
	bool IsInert() const;

	virtual void SetKeyValue(std::string key,std::string val);
	void MarkForSnapshot(bool b=true);
	bool IsMarkedForSnapshot() const;
	virtual void PrecacheModels();

	virtual void Initialize();
	unsigned int GetIndex() const;
	virtual uint32_t GetLocalIndex() const;
	uint64_t GetUniqueIndex() const;
	void SetUniqueIndex(uint64_t idx);

	virtual bool IsCharacter() const=0;
	virtual bool IsPlayer() const=0;
	virtual bool IsWeapon() const=0;
	virtual bool IsVehicle() const=0;
	virtual bool IsNPC() const=0;
	virtual bool IsWorld() const;
	virtual bool IsScripted() const;

	virtual Con::c_cout& print(Con::c_cout&);
	virtual std::ostream& print(std::ostream&);
	
	virtual void Remove();
	void RemoveSafely();

	void Spawn();
	virtual void OnSpawn();
	virtual void OnPostSpawn();
	bool IsSpawned() const;

	lua_State *GetLuaState() const;

	virtual void Load(DataStream &ds);
	virtual void Save(DataStream &ds);
	virtual BaseEntity *Copy();
protected:
	StateFlags m_stateFlags = StateFlags::None;

	// Transform component is needed frequently, so we store a direct reference to it for faster access
	std::weak_ptr<pragma::BaseTransformComponent> m_transformComponent = {};

	virtual void OnComponentAdded(pragma::BaseEntityComponent &component) override;
	virtual void OnComponentRemoved(pragma::BaseEntityComponent &component) override;

	// Should only be used by quick-access methods!
	// Adds the component and trasmits the information
	// to the clients if called serverside.
	virtual util::WeakHandle<pragma::BaseEntityComponent> AddNetworkedComponent(const std::string &name);
protected:
	uint32_t m_spawnFlags = 0u;

	std::vector<EntityHandle> m_entsRemove; // List of entities that should be removed when this entity is removed
	std::string m_class = "BaseEntity";
	uint32_t m_index = 0u;
	uint64_t m_uniqueIndex = 0ull;
	virtual void EraseFunction(int function);
	virtual void DoSpawn();
	pragma::NetEventId SetupNetEvent(const std::string &name) const;

	virtual void InitializeHandle() override;
	template<class THandle>
		void InitializeHandle();
};
REGISTER_BASIC_BITWISE_OPERATORS(BaseEntity::StateFlags);
#pragma warning(pop)

inline DLLNETWORK Con::c_cout& operator<<(Con::c_cout &os,BaseEntity &ent) {return ent.print(os);}

DLLNETWORK Con::c_cout& operator<<(Con::c_cout &os,const EntityHandle &ent);
DLLNETWORK std::ostream& operator<<(std::ostream &os,const EntityHandle ent);

DLLNETWORK bool operator==(const EntityHandle &a,const EntityHandle &b);

#endif