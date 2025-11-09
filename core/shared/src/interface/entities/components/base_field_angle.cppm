// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_field_angle;

export import :entities.components.base;

export namespace pragma {
	namespace baseFieldAngleComponent {
		CLASS_ENUM_COMPAT ComponentEventId EVENT_ON_FIELD_ANGLE_CHANGED;
	}
	class DLLNETWORK BaseFieldAngleComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		umath::Degree GetFieldAngle() const;
		const util::PFloatProperty &GetFieldAngleProperty() const;
		void SetFieldAngle(umath::Degree coneAngle);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseFieldAngleComponent(pragma::ecs::BaseEntity &ent);
		pragma::NetEventId m_netEvSetFieldAngle = pragma::INVALID_NET_EVENT;
		util::PFloatProperty m_fieldAngle;
	};
	struct DLLNETWORK CEOnFieldAngleChanged : public ComponentEvent {
		CEOnFieldAngleChanged(float oldRadius, float newRadius);
		virtual void PushArguments(lua::State *l) override;
		umath::Degree oldFieldAngle;
		umath::Degree newFieldAngle;
	};
};
