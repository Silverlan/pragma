// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.constraints.child_of;

export import :entities.components.constraints.base;
export import :entities.universal_reference;

export namespace pragma {
	class ConstraintComponent;
	class DLLNETWORK ConstraintChildOfComponent final : public BaseEntityComponent {
	  public:
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintChildOfComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetLocationAxisEnabled(Axis axis, bool enabled);
		bool IsLocationAxisEnabled(Axis axis) const;

		void SetRotationAxisEnabled(Axis axis, bool enabled);
		bool IsRotationAxisEnabled(Axis axis) const;

		void SetScaleAxisEnabled(Axis axis, bool enabled);
		bool IsScaleAxisEnabled(Axis axis) const;

		std::pair<std::optional<Vector3>, std::optional<Quat>> GetDriverPose(math::CoordinateSpace space) const;
		std::pair<std::optional<Vector3>, std::optional<Quat>> GetDrivenPose(math::CoordinateSpace space) const;

		const ComponentHandle<ConstraintComponent> &GetConstraint() const;

		virtual void InitializeLuaObject(lua::State *lua) override;
		std::optional<math::ScaledTransform> CalcInversePose(math::ScaledTransform &pose) const;
	  protected:
		enum class Type : uint8_t { Pose = 0, Position, Rotation };
		struct PropertyInfo {
			EntityUComponentMemberRef propertyRef;
			Type type;
		};
		std::pair<std::optional<Vector3>, std::optional<Quat>> GetPropertyPose(const PropertyInfo &propInfo, const BaseEntityComponent &c, math::CoordinateSpace space) const;

		static std::optional<EntityUComponentMemberRef> FindPosePropertyReference(const BaseEntityComponent &c, ComponentMemberIndex basePropIdx);
		void SetPropertyInfosDirty();
		bool UpdatePropertyInfos();
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		std::optional<math::ScaledTransform> CalcConstraintPose(math::ScaledTransform *optPose, bool inverse, ComponentMemberIndex &outDrivenPropertyIndex, ConstraintComponent::ConstraintParticipants &outConstraintParticipants) const;
		ComponentHandle<ConstraintComponent> m_constraintC;
		void UpdateAxisState();
		std::array<bool, 3> m_locationEnabled;
		std::array<bool, 3> m_rotationEnabled;
		std::array<bool, 3> m_scaleEnabled;
		std::optional<PropertyInfo> m_drivenObjectPropertyInfo {};
		std::optional<PropertyInfo> m_driverPropertyInfo {};
		bool m_allAxesEnabled = true;
	};
};
