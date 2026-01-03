// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.components.base_transform;

export import :entities.components.base;
export import :game.enums;

export {
	namespace pragma {
		enum class TransformChangeFlags : uint8_t { None = 0, PositionChanged = 1u, RotationChanged = PositionChanged << 1u, ScaleChanged = RotationChanged << 1u };
		struct DLLNETWORK CEOnPoseChanged : public ComponentEvent {
			CEOnPoseChanged(TransformChangeFlags changeFlags);
			virtual void PushArguments(lua::State *l) override;
			TransformChangeFlags changeFlags;
		};
		namespace baseTransformComponent {
			REGISTER_COMPONENT_EVENT(EVENT_ON_POSE_CHANGED)
			REGISTER_COMPONENT_EVENT(EVENT_ON_TELEPORT)
		}
		class DLLNETWORK BaseTransformComponent : public BaseEntityComponent {
		  public:
			static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
			static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
			virtual void Initialize() override;

			void SetPosition(const Vector3 &pos, CoordinateSpace space);
			void SetPosition(const Vector3 &pos);
			const Vector3 &GetPosition() const;
			Vector3 GetPosition(CoordinateSpace space) const;
			void SetRotation(const Quat &q, CoordinateSpace space);
			void SetRotation(const Quat &q);
			const Quat &GetRotation() const;
			Quat GetRotation(CoordinateSpace space) const;
			void SetPose(const math::ScaledTransform &pose, CoordinateSpace space);
			void SetPose(const math::Transform &pose, CoordinateSpace space);
			void SetPose(const math::ScaledTransform &pose);
			void SetPose(const math::Transform &pose);
			const math::ScaledTransform &GetPose() const;
			math::ScaledTransform GetPose(CoordinateSpace space) const;
			void SetAngles(const EulerAngles &ang, CoordinateSpace space);
			void SetAngles(const EulerAngles &ang);
			void SetPitch(float pitch, CoordinateSpace space = CoordinateSpace::World);
			void SetYaw(float yaw, CoordinateSpace space = CoordinateSpace::World);
			void SetRoll(float roll, CoordinateSpace space = CoordinateSpace::World);
			EulerAngles GetAngles(CoordinateSpace space) const;
			EulerAngles GetAngles() const;
			float GetPitch(CoordinateSpace space = CoordinateSpace::World) const;
			float GetYaw(CoordinateSpace space = CoordinateSpace::World) const;
			float GetRoll(CoordinateSpace space = CoordinateSpace::World) const;
			Mat4 GetRotationMatrix(CoordinateSpace space = CoordinateSpace::World) const;
			Vector3 GetForward(CoordinateSpace space = CoordinateSpace::World) const;
			Vector3 GetUp(CoordinateSpace space = CoordinateSpace::World) const;
			Vector3 GetRight(CoordinateSpace space = CoordinateSpace::World) const;
			float GetDistance(const Vector3 &p) const;
			float GetDistance(const ecs::BaseEntity &ent) const;
			void GetOrientation(Vector3 *forward, Vector3 *right, Vector3 *up = nullptr) const;

			void Teleport(const math::Transform &targetPose);

			void LocalToWorld(Vector3 *origin) const;
			void LocalToWorld(Quat *rot) const;
			void LocalToWorld(Vector3 *origin, Quat *rot) const;

			void WorldToLocal(Vector3 *origin) const;
			void WorldToLocal(Quat *rot) const;
			void WorldToLocal(Vector3 *origin, Quat *rot) const;

			Vector3 GetOrigin() const;

			Vector3 GetDirection(const ecs::BaseEntity &ent, bool bIgnoreYAxis = false) const;
			EulerAngles GetAngles(const ecs::BaseEntity &ent, bool bIgnoreYAxis = false) const;
			float GetDotProduct(const ecs::BaseEntity &ent, bool bIgnoreYAxis = false) const;
			Vector3 GetDirection(const Vector3 &pos, bool bIgnoreYAxis = false) const;
			EulerAngles GetAngles(const Vector3 &pos, bool bIgnoreYAxis = false) const;
			float GetDotProduct(const Vector3 &pos, bool bIgnoreYAxis = false) const;

			float GetMaxAxisScale() const;
			float GetAbsMaxAxisScale() const;
			const Vector3 &GetScale() const;
			Vector3 GetScale(CoordinateSpace space) const;
			void SetScale(float scale);
			void SetScale(float scale, CoordinateSpace space);
			virtual void SetScale(const Vector3 &scale);
			void SetScale(const Vector3 &scale, CoordinateSpace space);

			virtual Vector3 GetEyePosition() const;
			Vector3 GetEyeOffset() const;
			virtual void SetEyeOffset(const Vector3 &offset);

			double GetLastMoveTime() const;

			void SetPosition(const Vector3 &pos, Bool bForceUpdate, CoordinateSpace space = CoordinateSpace::World);

			virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
			virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

			// Same as SetPosition / SetRotation / SetScale, but don't invoke callbacks
			void SetRawPosition(const Vector3 &pos, CoordinateSpace space = CoordinateSpace::World);
			void SetRawRotation(const Quat &rot, CoordinateSpace space = CoordinateSpace::World);
			void SetRawScale(const Vector3 &scale, CoordinateSpace space = CoordinateSpace::World);

			void UpdateLastMovedTime();
			void OnPoseChanged(TransformChangeFlags changeFlags, bool updatePhysics = true);
		  protected:
			BaseTransformComponent(ecs::BaseEntity &ent);
			NetEventId m_netEvSetScale = INVALID_NET_EVENT;
			double m_tLastMoved = 0.0; // Last time the entity moved or changed rotation
			Vector3 m_eyeOffset = {};
			math::ScaledTransform m_pose {};
		};
		struct DLLNETWORK CETeleport : public ComponentEvent {
			CETeleport(const math::Transform &originalPose, const math::Transform &targetPose, const math::Transform &deltaPose);
			virtual void PushArguments(lua::State *l) override;
			math::Transform originalPose;
			math::Transform targetPose;
			math::Transform deltaPose;
		};
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::TransformChangeFlags)
};
