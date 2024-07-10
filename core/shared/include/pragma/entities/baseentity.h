/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASEENTITY_H__
#define __BASEENTITY_H__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/base_lua_handle.hpp"
#include "pragma/entities/entity_component_system.hpp"
#include "pragma/util/global_string_table.hpp"
#include "pragma/util/coordinate_space.hpp"
#include "pragma/model/animation/play_animation_flags.hpp"
#include "pragma/types.hpp"
#include <pragma/console/conout.h>
#include <sharedutils/util_shared_handle.hpp>
#ifdef _WIN32
#include <format>
#endif

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

namespace util {
	using Uuid = std::array<uint64_t, 2>;
};

namespace pragma {
	class BaseEntityComponent;
	class BaseModelComponent;
	class BaseGenericComponent;
	class BaseChildComponent;
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
	class ParentComponent;
	struct EntityUComponentMemberRef;

	using NetEventId = uint32_t;

	namespace ents {
		DLLNETWORK const char *register_class_name(const std::string &className);
	};
};

namespace umath {
	class Transform;
	class ScaledTransform;
};

const double ENT_EPSILON = 0.000'01;

class DataStream;
using EntityIndex = uint32_t;
namespace udm {
	struct LinkedPropertyWrapper;
};
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLNETWORK BaseEntity : public pragma::BaseLuaHandle, public pragma::BaseEntityComponentSystem {
  public:
	static pragma::ComponentEventId EVENT_HANDLE_KEY_VALUE;
	static pragma::ComponentEventId EVENT_ON_SPAWN;
	static pragma::ComponentEventId EVENT_ON_POST_SPAWN;
	static pragma::ComponentEventId EVENT_ON_REMOVE;
	static constexpr auto PSAVE_IDENTIFIER = "PSAVE";
	static constexpr uint32_t PSAVE_VERSION = 1;

	enum class StateFlags : uint16_t {
		None = 0u,
		Spawned = 1u,
		SnapshotUpdateRequired = Spawned << 1u,

		PositionChanged = SnapshotUpdateRequired << 1u,
		RotationChanged = PositionChanged << 1u,
		CollisionBoundsChanged = RotationChanged << 1u,
		RenderBoundsChanged = CollisionBoundsChanged << 1u,

		HasWorldComponent = RenderBoundsChanged << 1u,
		Removed = HasWorldComponent << 1u,
		IsSpawning = Removed << 1u
	};

	static void RegisterEvents(pragma::EntityComponentManager &componentManager);

	virtual pragma::GString GetClass() const;
	BaseEntity();
	void Construct(unsigned int idx);
	EntityHandle GetHandle() const;

	const util::Uuid GetUuid() const { return m_uuid; }
	void SetUuid(const util::Uuid &uuid);

	friend Engine;
  public:
	StateFlags GetStateFlags() const;
	bool HasStateFlag(StateFlags flag) const;
	void SetStateFlag(StateFlags flag);
	void ResetStateChangeFlags();

	virtual bool IsStatic() const;
	bool IsDynamic() const;
	virtual NetworkState *GetNetworkState() const = 0;
	Game &GetGame() const;

	pragma::NetEventId FindNetEvent(const std::string &name) const;

	// Returns IDENTITY if the entity has no transform component
	const umath::ScaledTransform &GetPose() const;
	umath::ScaledTransform GetPose(pragma::CoordinateSpace space) const;
	void SetPose(const umath::ScaledTransform &outTransform, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
	void SetPose(const umath::Transform &outTransform, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
	const Vector3 &GetPosition() const;
	Vector3 GetPosition(pragma::CoordinateSpace space) const;
	void SetPosition(const Vector3 &pos, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);
	Vector3 GetCenter() const;

	// Returns unit quaternion if entity has no transform component
	const Quat &GetRotation() const;
	Quat GetRotation(pragma::CoordinateSpace space) const;
	void SetRotation(const Quat &rot, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);

	const Vector3 &GetScale() const;
	Vector3 GetScale(pragma::CoordinateSpace space) const;
	void SetScale(const Vector3 &scale, pragma::CoordinateSpace space = pragma::CoordinateSpace::World);

	pragma::BaseEntityComponent *FindComponentMemberIndex(const util::Path &path, pragma::ComponentMemberIndex &outMemberIdx);
	const pragma::BaseEntityComponent *FindComponentMemberIndex(const util::Path &path, pragma::ComponentMemberIndex &outMemberIdx) const { return const_cast<BaseEntity *>(this)->FindComponentMemberIndex(path, outMemberIdx); }

	// Helper functions
	virtual pragma::ComponentHandle<pragma::BaseAnimatedComponent> GetAnimatedComponent() const = 0;
	virtual pragma::ComponentHandle<pragma::BaseWeaponComponent> GetWeaponComponent() const = 0;
	virtual pragma::ComponentHandle<pragma::BaseVehicleComponent> GetVehicleComponent() const = 0;
	virtual pragma::ComponentHandle<pragma::BaseAIComponent> GetAIComponent() const = 0;
	virtual pragma::ComponentHandle<pragma::BaseCharacterComponent> GetCharacterComponent() const = 0;
	virtual pragma::ComponentHandle<pragma::BasePlayerComponent> GetPlayerComponent() const = 0;
	virtual pragma::ComponentHandle<pragma::BaseTimeScaleComponent> GetTimeScaleComponent() const = 0;
	virtual pragma::ComponentHandle<pragma::BaseNameComponent> GetNameComponent() const = 0;
	pragma::BaseModelComponent *GetModelComponent() const;
	pragma::BaseTransformComponent *GetTransformComponent() const;
	pragma::BasePhysicsComponent *GetPhysicsComponent() const;
	pragma::BaseGenericComponent *GetGenericComponent() const;
	pragma::BaseChildComponent *GetChildComponent() const;

	// These are quick-access functions for commonly used component functions.
	// In some cases these may create the component, if it doesn't exist, and transmit
	// it to the client if called serverside!
	std::shared_ptr<ALSound> CreateSound(const std::string &snd, ALSoundType type);
	std::shared_ptr<ALSound> EmitSound(const std::string &snd, ALSoundType type, float gain = 1.f, float pitch = 1.f);

	std::string GetName() const;
	void SetName(const std::string &name);

	void SetModel(const std::string &mdl);
	void SetModel(const std::shared_ptr<Model> &mdl);
	const std::shared_ptr<Model> &GetModel() const;
	std::string GetModelName() const;
	std::optional<umath::Transform> GetAttachmentPose(uint32_t attId) const;
	uint32_t GetSkin() const;
	void SetSkin(uint32_t skin);
	uint32_t GetBodyGroup(const std::string &name) const;
	void SetBodyGroup(const std::string &name, uint32_t id);

	BaseEntity *CreateChild(const std::string &className);
	void SetParent(BaseEntity *parent);
	void ClearParent();
	BaseEntity *GetParent() const;
	bool HasParent() const;
	bool HasChildren() const;

	bool IsChildOf(const BaseEntity &ent) const;
	bool IsAncestorOf(const BaseEntity &ent) const;
	bool IsDescendantOf(const BaseEntity &ent) const;
	bool IsParentOf(const BaseEntity &ent) const;

	PhysObj *GetPhysicsObject() const;
	PhysObj *InitializePhysics(PHYSICSTYPE type);
	void DestroyPhysicsObject();
	void DropToFloor();
	std::pair<Vector3, Vector3> GetCollisionBounds() const;
	void SetCollisionFilterMask(CollisionMask filterMask);
	void SetCollisionFilterGroup(CollisionMask filterGroup);
	CollisionMask GetCollisionFilterGroup() const;
	CollisionMask GetCollisionFilterMask() const;

	void SetEnabled(bool enabled);
	bool IsEnabled() const;
	bool IsDisabled() const;

	std::optional<Color> GetColor() const;
	void SetColor(const Color &color);

	Vector3 GetForward() const;
	Vector3 GetUp() const;
	Vector3 GetRight() const;

	void Input(const std::string &input, BaseEntity *activator = nullptr, BaseEntity *caller = nullptr, const std::string &data = "");

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

	void PlayAnimation(int32_t animation, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
	void PlayLayeredAnimation(int32_t slot, int32_t animation, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
	bool PlayActivity(Activity activity, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
	bool PlayLayeredActivity(int32_t slot, Activity activity, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
	bool PlayLayeredAnimation(int32_t slot, std::string animation, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
	void StopLayeredAnimation(int slot);
	bool PlayAnimation(const std::string &animation, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
	int32_t GetAnimation() const;
	Activity GetActivity() const;

	void TakeDamage(DamageInfo &info);
	//

	// Returns true if this entity is local to the current network state (i.e. clientside/serverside only)
	virtual bool IsNetworkLocal() const = 0;

	void SetSpawnFlags(uint32_t spawnFlags);
	unsigned int GetSpawnFlags() const;

	void RemoveEntityOnRemoval(BaseEntity *ent, Bool bRemove = true);
	void RemoveEntityOnRemoval(const EntityHandle &hEnt, Bool bRemove = true);

	CallbackHandle CallOnRemove(const CallbackHandle &hCallback);

	bool IsMapEntity() const;

	virtual void OnRemove() override;
	// Returns true if the entity is static, or asleep (and not animated)
	bool IsInert() const;

	virtual void SetKeyValue(std::string key, std::string val);
	void MarkForSnapshot(bool b = true);
	bool IsMarkedForSnapshot() const;
	virtual void PrecacheModels();

	virtual void Initialize();
	EntityIndex GetIndex() const;
	virtual uint32_t GetLocalIndex() const;

	virtual bool IsCharacter() const = 0;
	virtual bool IsPlayer() const = 0;
	virtual bool IsWeapon() const = 0;
	virtual bool IsVehicle() const = 0;
	virtual bool IsNPC() const = 0;
	bool IsWorld() const;
	virtual bool IsScripted() const;

	virtual Con::c_cout &print(Con::c_cout &);
	virtual std::ostream &print(std::ostream &);
	std::string ToString() const;

	bool IsRemoved() const;
	virtual void Remove();
	void RemoveSafely();

	void Spawn();
	virtual void OnSpawn();
	virtual void OnPostSpawn();
	bool IsSpawned() const;

	lua_State *GetLuaState() const;

	virtual void Load(udm::LinkedPropertyWrapper &udm);
	virtual void Save(udm::LinkedPropertyWrapper &udm);
	virtual BaseEntity *Copy();

	std::string GetUri() const;
	static std::string GetUri(util::Uuid uuid);
	static std::string GetUri(const std::string name);
	static bool ParseUri(std::string uriPath, pragma::EntityUComponentMemberRef &outRef, const util::Uuid *optSelf = nullptr);
	static bool CreateMemberReference(pragma::EntityIdentifier identifier, std::string var, pragma::EntityUComponentMemberRef &outRef);
  protected:
	StateFlags m_stateFlags = StateFlags::None;

	// These components are needed frequently, so we store a direct reference to them for faster access
	pragma::BaseTransformComponent *m_transformComponent = nullptr;
	pragma::BasePhysicsComponent *m_physicsComponent = nullptr;
	pragma::BaseModelComponent *m_modelComponent = nullptr;
	pragma::BaseGenericComponent *m_genericComponent = nullptr;
	pragma::BaseChildComponent *m_childComponent = nullptr;

	// Should only be used by quick-access methods!
	// Adds the component and trasmits the information
	// to the clients if called serverside.
	virtual pragma::ComponentHandle<pragma::BaseEntityComponent> AddNetworkedComponent(const std::string &name);
  protected:
	uint32_t m_spawnFlags = 0u;

	pragma::GString m_className = "BaseEntity";
	util::Uuid m_uuid {};
	EntityIndex m_index = 0u;
	virtual void DoSpawn();
	pragma::NetEventId SetupNetEvent(const std::string &name) const;
};
REGISTER_BASIC_BITWISE_OPERATORS(BaseEntity::StateFlags);
#pragma warning(pop)

inline DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, BaseEntity &ent) { return ent.print(os); }

DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const EntityHandle &ent);
DLLNETWORK std::ostream &operator<<(std::ostream &os, const EntityHandle ent);

DLLNETWORK bool operator==(const EntityHandle &a, const EntityHandle &b);

#ifdef _WIN32
template<>
struct std::formatter<BaseEntity> : std::formatter<std::string> {
	auto format(BaseEntity &ent, format_context &ctx) -> decltype(ctx.out())
	{
		std::stringstream ss;
		ent.print(ss);
		return std::format_to(ctx.out(), "{}", ss.str());
	}
};
#endif

#include "pragma/lua/converters/entity_converter.hpp"

#endif
