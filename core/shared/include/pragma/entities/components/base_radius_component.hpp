// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_RADIUS_COMPONENT_HPP__
#define __BASE_RADIUS_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma {
	class DLLNETWORK BaseRadiusComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_RADIUS_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;
		float GetRadius() const;
		const util::PFloatProperty &GetRadiusProperty() const;
		void SetRadius(float radius);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseRadiusComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvSetRadius = pragma::INVALID_NET_EVENT;
		util::PFloatProperty m_radius;
	};
	struct DLLNETWORK CEOnRadiusChanged : public ComponentEvent {
		CEOnRadiusChanged(float oldRadius, float newRadius);
		virtual void PushArguments(lua_State *l) override;
		float oldRadius;
		float newRadius;
	};
};

#endif
