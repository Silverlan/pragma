// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_field_angle;

export import :entities.components.base;

export namespace pragma {
	namespace baseFieldAngleComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_FIELD_ANGLE_CHANGED)
	}
	class DLLNETWORK BaseFieldAngleComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		math::Degree GetFieldAngle() const;
		const util::PFloatProperty &GetFieldAngleProperty() const;
		void SetFieldAngle(math::Degree coneAngle);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseFieldAngleComponent(ecs::BaseEntity &ent);
		NetEventId m_netEvSetFieldAngle = INVALID_NET_EVENT;
		util::PFloatProperty m_fieldAngle;
	};
	struct DLLNETWORK CEOnFieldAngleChanged : public ComponentEvent {
		CEOnFieldAngleChanged(float oldRadius, float newRadius);
		virtual void PushArguments(lua::State *l) override;
		math::Degree oldFieldAngle;
		math::Degree newFieldAngle;
	};
};
