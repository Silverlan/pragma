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
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);

		ConstraintChildOfComponent(pragma::ecs::BaseEntity &ent);
		virtual void Initialize() override;

		void SetLocationAxisEnabled(pragma::Axis axis, bool enabled);
		bool IsLocationAxisEnabled(pragma::Axis axis) const;

		void SetRotationAxisEnabled(pragma::Axis axis, bool enabled);
		bool IsRotationAxisEnabled(pragma::Axis axis) const;

		void SetScaleAxisEnabled(pragma::Axis axis, bool enabled);
		bool IsScaleAxisEnabled(pragma::Axis axis) const;

		std::pair<std::optional<Vector3>, std::optional<Quat>> GetDriverPose(umath::CoordinateSpace space) const;
		std::pair<std::optional<Vector3>, std::optional<Quat>> GetDrivenPose(umath::CoordinateSpace space) const;

		const pragma::ComponentHandle<ConstraintComponent> &GetConstraint() const;

		virtual void InitializeLuaObject(lua::State *lua) override;
		std::optional<umath::ScaledTransform> CalcInversePose(umath::ScaledTransform &pose) const;
	  protected:
		enum class Type : uint8_t { Pose = 0, Position, Rotation };
		struct PropertyInfo {
			pragma::EntityUComponentMemberRef propertyRef;
			Type type;
		};
		std::pair<std::optional<Vector3>, std::optional<Quat>> GetPropertyPose(const PropertyInfo &propInfo, const BaseEntityComponent &c, umath::CoordinateSpace space) const;

		static std::optional<pragma::EntityUComponentMemberRef> FindPosePropertyReference(const pragma::BaseEntityComponent &c, pragma::ComponentMemberIndex basePropIdx);
		void SetPropertyInfosDirty();
		bool UpdatePropertyInfos();
		void ApplyConstraint();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		std::optional<umath::ScaledTransform> CalcConstraintPose(umath::ScaledTransform *optPose, bool inverse, pragma::ComponentMemberIndex &outDrivenPropertyIndex, ConstraintComponent::ConstraintParticipants &outConstraintParticipants) const;
		pragma::ComponentHandle<ConstraintComponent> m_constraintC;
		void UpdateAxisState();
		std::array<bool, 3> m_locationEnabled;
		std::array<bool, 3> m_rotationEnabled;
		std::array<bool, 3> m_scaleEnabled;
		std::optional<PropertyInfo> m_drivenObjectPropertyInfo {};
		std::optional<PropertyInfo> m_driverPropertyInfo {};
		bool m_allAxesEnabled = true;
	};
};
