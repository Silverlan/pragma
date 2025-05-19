/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_FLAMMABLE_COMPONENT_HPP__
#define __BASE_FLAMMABLE_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma {
	class DLLNETWORK BaseFlammableComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_IGNITED;
		static ComponentEventId EVENT_ON_EXTINGUISHED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		virtual ~BaseFlammableComponent() override;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		const util::PBoolProperty &GetOnFireProperty() const;
		const util::PBoolProperty &GetIgnitableProperty() const;
		bool IsOnFire() const;
		bool IsIgnitable() const;
		virtual util::EventReply Ignite(float duration, BaseEntity *attacker = nullptr, BaseEntity *inflictor = nullptr);
		virtual void Extinguish();
		virtual void SetIgnitable(bool b);
		virtual void OnTick(double dt) override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseFlammableComponent(BaseEntity &ent);
		pragma::NetEventId m_netEvIgnite = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvExtinguish = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetIgnitable = pragma::INVALID_NET_EVENT;
		util::PBoolProperty m_bIsOnFire;
		util::PBoolProperty m_bIgnitable;
		float m_tExtinguishTime = 0.f;
	};
	struct DLLNETWORK CEOnIgnited : public ComponentEvent {
		CEOnIgnited(float duration, BaseEntity *attacker, BaseEntity *inflictor);
		virtual void PushArguments(lua_State *l) override;
		float duration;
		EntityHandle attacker;
		EntityHandle inflictor;
	};
};

#endif
