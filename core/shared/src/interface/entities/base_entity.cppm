// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <format>

#include "pragma/lua/core.hpp"


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

			friend pragma::Engine;
		public:
			StateFlags GetStateFlags() const;
			bool HasStateFlag(StateFlags flag) const;
			void SetStateFlag(StateFlags flag);
			void ResetStateChangeFlags();

			virtual bool IsStatic() const;
			bool IsDynamic() const;
			virtual NetworkState *GetNetworkState() const = 0;
			pragma::Game &GetGame() const;

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
			const pragma::BaseEntityComponent *FindComponentMemberIndex(const util::Path &path, pragma::ComponentMemberIndex &outMemberIdx) const { return const_cast<pragma::ecs::BaseEntity *>(this)->FindComponentMemberIndex(path, outMemberIdx); }

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
			std::shared_ptr<ALSound> CreateSound(const std::string &snd, pragma::audio::ALSoundType type);
			std::shared_ptr<ALSound> EmitSound(const std::string &snd, pragma::audio::ALSoundType type, float gain = 1.f, float pitch = 1.f);

			std::string GetName() const;
			void SetName(const std::string &name);

			void SetModel(const std::string &mdl);
			void SetModel(const std::shared_ptr<pragma::Model> &mdl);
			const std::shared_ptr<pragma::Model> &GetModel() const;
			std::string GetModelName() const;
			std::optional<umath::Transform> GetAttachmentPose(uint32_t attId) const;
			uint32_t GetSkin() const;
			void SetSkin(uint32_t skin);
			uint32_t GetBodyGroup(const std::string &name) const;
			void SetBodyGroup(const std::string &name, uint32_t id);

			pragma::ecs::BaseEntity *CreateChild(const std::string &className);
			void SetParent(pragma::ecs::BaseEntity *parent);
			void ClearParent();
			pragma::ecs::BaseEntity *GetParent() const;
			bool HasParent() const;
			bool HasChildren() const;

			bool IsChildOf(const pragma::ecs::BaseEntity &ent) const;
			bool IsAncestorOf(const pragma::ecs::BaseEntity &ent) const;
			bool IsDescendantOf(const pragma::ecs::BaseEntity &ent) const;
			bool IsParentOf(const pragma::ecs::BaseEntity &ent) const;

			pragma::physics::PhysObj *GetPhysicsObject() const;
			pragma::physics::PhysObj *InitializePhysics(pragma::physics::PHYSICSTYPE type);
			void DestroyPhysicsObject();
			void DropToFloor();
			std::pair<Vector3, Vector3> GetCollisionBounds() const;
			void SetCollisionFilterMask(pragma::physics::CollisionMask filterMask);
			void SetCollisionFilterGroup(pragma::physics::CollisionMask filterGroup);
			pragma::physics::CollisionMask GetCollisionFilterGroup() const;
			pragma::physics::CollisionMask GetCollisionFilterMask() const;

			void SetEnabled(bool enabled);
			bool IsEnabled() const;
			bool IsDisabled() const;

			std::optional<Color> GetColor() const;
			void SetColor(const Color &color);

			Vector3 GetForward() const;
			Vector3 GetUp() const;
			Vector3 GetRight() const;

			void Input(const std::string &input, pragma::ecs::BaseEntity *activator = nullptr, pragma::ecs::BaseEntity *caller = nullptr, const std::string &data = "");

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
			bool PlayActivity(pragma::Activity activity, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
			bool PlayLayeredActivity(int32_t slot, pragma::Activity activity, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
			bool PlayLayeredAnimation(int32_t slot, std::string animation, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
			void StopLayeredAnimation(int slot);
			bool PlayAnimation(const std::string &animation, pragma::FPlayAnim flags = pragma::FPlayAnim::Default);
			int32_t GetAnimation() const;
			pragma::Activity GetActivity() const;

			void TakeDamage(DamageInfo &info);
			//

			// Returns true if this entity is local to the current network state (i.e. clientside/serverside only)
			virtual bool IsNetworkLocal() const = 0;

			void SetSpawnFlags(uint32_t spawnFlags);
			unsigned int GetSpawnFlags() const;

			void RemoveEntityOnRemoval(pragma::ecs::BaseEntity *ent, Bool bRemove = true);
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

			lua_State *GetLuaState() const;

			virtual void Load(udm::LinkedPropertyWrapper &udm);
			virtual void Save(udm::LinkedPropertyWrapper &udm);
			virtual pragma::ecs::BaseEntity *Copy();

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
    	using namespace umath::scoped_enum::bitwise;

		inline DLLNETWORK Con::c_cout &operator<<(Con::c_cout &os, const BaseEntity &ent) { return ent.print(os); }

		inline std::ostream& operator<<(std::ostream& os, const BaseEntity& ent) {
			const_cast<BaseEntity&>(ent).print(os);
			return os;
		}
		using ::operator<<;
	}
	namespace umath::scoped_enum::bitwise {
		template<>
		struct enable_bitwise_operators<pragma::ecs::BaseEntity::StateFlags> : std::true_type {};
	}
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
