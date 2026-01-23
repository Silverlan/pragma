// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"
#include <format>

export module pragma.shared:entities.base_entity;

export import :audio.enums;
export import :audio.sound;
import :console.output;
export import :entities.base_entity_handle;
export import :entities.components.handle;
export import :entities.manager;
export import :entities.net_event_manager;
export import :entities.system;
export import :game.enums;
export import :model.animation.enums;
export import :physics.enums;
export import :scripting.lua.base_lua_handle;
export import :types;

export import :util.global_string_table;

export {
	namespace pragma {
		namespace ents {
			DLLNETWORK const char *register_class_name(const std::string &className);
		};
	};

	const double ENT_EPSILON = 0.000'01;

#pragma warning(push)
#pragma warning(disable : 4251)
	namespace pragma::ecs {
		namespace baseEntity {
			REGISTER_COMPONENT_EVENT(EVENT_HANDLE_KEY_VALUE)
			REGISTER_COMPONENT_EVENT(EVENT_ON_SPAWN)
			REGISTER_COMPONENT_EVENT(EVENT_ON_POST_SPAWN)
			REGISTER_COMPONENT_EVENT(EVENT_ON_REMOVE)
		}
		class DLLNETWORK BaseEntity : public BaseLuaHandle, public BaseEntityComponentSystem {
		  public:
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

			static void RegisterEvents(EntityComponentManager &componentManager);

			virtual GString GetClass() const;
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

			NetEventId FindNetEvent(const std::string &name) const;

			// Returns IDENTITY if the entity has no transform component
			const math::ScaledTransform &GetPose() const;
			math::ScaledTransform GetPose(CoordinateSpace space) const;
			void SetPose(const math::ScaledTransform &outTransform, CoordinateSpace space = CoordinateSpace::World);
			void SetPose(const math::Transform &outTransform, CoordinateSpace space = CoordinateSpace::World);
			const Vector3 &GetPosition() const;
			Vector3 GetPosition(CoordinateSpace space) const;
			void SetPosition(const Vector3 &pos, CoordinateSpace space = CoordinateSpace::World);
			Vector3 GetCenter() const;

			// Returns unit quaternion if entity has no transform component
			const Quat &GetRotation() const;
			Quat GetRotation(CoordinateSpace space) const;
			void SetRotation(const Quat &rot, CoordinateSpace space = CoordinateSpace::World);

			const Vector3 &GetScale() const;
			Vector3 GetScale(CoordinateSpace space) const;
			void SetScale(const Vector3 &scale, CoordinateSpace space = CoordinateSpace::World);

			BaseEntityComponent *FindComponentMemberIndex(const util::Path &path, ComponentMemberIndex &outMemberIdx);
			const BaseEntityComponent *FindComponentMemberIndex(const util::Path &path, ComponentMemberIndex &outMemberIdx) const { return const_cast<BaseEntity *>(this)->FindComponentMemberIndex(path, outMemberIdx); }

			// Helper functions
			virtual ComponentHandle<BaseAnimatedComponent> GetAnimatedComponent() const = 0;
			virtual ComponentHandle<BaseWeaponComponent> GetWeaponComponent() const = 0;
			virtual ComponentHandle<BaseVehicleComponent> GetVehicleComponent() const = 0;
			virtual ComponentHandle<BaseAIComponent> GetAIComponent() const = 0;
			virtual ComponentHandle<BaseCharacterComponent> GetCharacterComponent() const = 0;
			virtual ComponentHandle<BasePlayerComponent> GetPlayerComponent() const = 0;
			virtual ComponentHandle<BaseTimeScaleComponent> GetTimeScaleComponent() const = 0;
			virtual ComponentHandle<BaseNameComponent> GetNameComponent() const = 0;
			BaseModelComponent *GetModelComponent() const;
			BaseTransformComponent *GetTransformComponent() const;
			BasePhysicsComponent *GetPhysicsComponent() const;
			BaseGenericComponent *GetGenericComponent() const;
			BaseChildComponent *GetChildComponent() const;

			// These are quick-access functions for commonly used component functions.
			// In some cases these may create the component, if it doesn't exist, and transmit
			// it to the client if called serverside!
			std::shared_ptr<audio::ALSound> CreateSound(const std::string &snd, audio::ALSoundType type);
			std::shared_ptr<audio::ALSound> EmitSound(const std::string &snd, audio::ALSoundType type, float gain = 1.f, float pitch = 1.f);

			std::string GetName() const;
			void SetName(const std::string &name);

			void SetModel(const std::string &mdl);
			void SetModel(const std::shared_ptr<asset::Model> &mdl);
			const std::shared_ptr<asset::Model> &GetModel() const;
			std::string GetModelName() const;
			std::optional<math::Transform> GetAttachmentPose(uint32_t attId) const;
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

			physics::PhysObj *GetPhysicsObject() const;
			physics::PhysObj *InitializePhysics(physics::PhysicsType type);
			void DestroyPhysicsObject();
			void DropToFloor();
			std::pair<Vector3, Vector3> GetCollisionBounds() const;
			void SetCollisionFilterMask(physics::CollisionMask filterMask);
			void SetCollisionFilterGroup(physics::CollisionMask filterGroup);
			physics::CollisionMask GetCollisionFilterGroup() const;
			physics::CollisionMask GetCollisionFilterMask() const;

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

			void PlayAnimation(int32_t animation, FPlayAnim flags = FPlayAnim::Default);
			void PlayLayeredAnimation(int32_t slot, int32_t animation, FPlayAnim flags = FPlayAnim::Default);
			bool PlayActivity(Activity activity, FPlayAnim flags = FPlayAnim::Default);
			bool PlayLayeredActivity(int32_t slot, Activity activity, FPlayAnim flags = FPlayAnim::Default);
			bool PlayLayeredAnimation(int32_t slot, std::string animation, FPlayAnim flags = FPlayAnim::Default);
			void StopLayeredAnimation(int slot);
			bool PlayAnimation(const std::string &animation, FPlayAnim flags = FPlayAnim::Default);
			int32_t GetAnimation() const;
			Activity GetActivity() const;

			void TakeDamage(game::DamageInfo &info);
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

			virtual Con::c_cout &print(Con::c_cout &) const;
			virtual std::ostream &print(std::ostream &) const;
			std::string ToString() const;

			bool IsRemoved() const;
			virtual void Remove();
			void RemoveSafely();

			void Spawn();
			virtual void OnSpawn();
			virtual void OnPostSpawn();
			bool IsSpawned() const;

			lua::State *GetLuaState() const;

			virtual void Load(udm::LinkedPropertyWrapper &udm);
			virtual void Save(udm::LinkedPropertyWrapper &udm);
			virtual BaseEntity *Copy();

			std::string GetUri() const;
			static std::string GetUri(util::Uuid uuid);
			static std::string GetUri(const std::string name);
			static bool ParseUri(std::string uriPath, EntityUComponentMemberRef &outRef, const util::Uuid *optSelf = nullptr);
			static bool CreateMemberReference(EntityIdentifier identifier, std::string var, EntityUComponentMemberRef &outRef);
		  protected:
			StateFlags m_stateFlags = StateFlags::None;

			// These components are needed frequently, so we store a direct reference to them for faster access
			BaseTransformComponent *m_transformComponent = nullptr;
			BasePhysicsComponent *m_physicsComponent = nullptr;
			BaseModelComponent *m_modelComponent = nullptr;
			BaseGenericComponent *m_genericComponent = nullptr;
			BaseChildComponent *m_childComponent = nullptr;

			// Should only be used by quick-access methods!
			// Adds the component and trasmits the information
			// to the clients if called serverside.
			virtual ComponentHandle<BaseEntityComponent> AddNetworkedComponent(const std::string &name);
		  protected:
			uint32_t m_spawnFlags = 0u;

			GString m_className = "BaseEntity";
			util::Uuid m_uuid {};
			EntityIndex m_index = 0u;
			virtual void DoSpawn();
			NetEventId SetupNetEvent(const std::string &name) const;
		};
		using namespace pragma::math::scoped_enum::bitwise;

		inline DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const BaseEntity &ent) { return ent.print(os); }

		inline std::ostream &operator<<(std::ostream &os, const BaseEntity &ent)
		{
			const_cast<BaseEntity &>(ent).print(os);
			return os;
		}
		using ::operator<<;
	}
	REGISTER_ENUM_FLAGS(pragma::ecs::BaseEntity::StateFlags)

#pragma warning(pop)

	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const EntityHandle &ent);
	DLLNETWORK std::ostream &operator<<(std::ostream &os, const EntityHandle ent);

	template<>
	struct std::formatter<pragma::ecs::BaseEntity> : std::formatter<std::string> {
		auto format(pragma::ecs::BaseEntity &ent, format_context &ctx) -> decltype(ctx.out())
		{
			std::stringstream ss;
			ent.print(ss);
			return std::format_to(ctx.out(), "{}", ss.str());
		}
	};
};
