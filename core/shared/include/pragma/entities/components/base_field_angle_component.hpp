/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_CONE_COMPONENT_HPP__
#define __BASE_CONE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma {
	class DLLNETWORK BaseFieldAngleComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_FIELD_ANGLE_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		umath::Degree GetFieldAngle() const;
		const util::PFloatProperty &GetFieldAngleProperty() const;
		void SetFieldAngle(umath::Degree coneAngle);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseFieldAngleComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetFieldAngle = pragma::INVALID_NET_EVENT;
		util::PFloatProperty m_fieldAngle;
	};
	struct DLLNETWORK CEOnFieldAngleChanged : public ComponentEvent {
		CEOnFieldAngleChanged(float oldRadius, float newRadius);
		virtual void PushArguments(lua_State *l) override;
		umath::Degree oldFieldAngle;
		umath::Degree newFieldAngle;
	};
};

#endif
