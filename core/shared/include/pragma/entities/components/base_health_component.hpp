/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_HEALTH_COMPONENT_HPP__
#define __BASE_HEALTH_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

class DamageInfo;
namespace pragma
{
	class DLLNETWORK BaseHealthComponent
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_TAKEN_DAMAGE;
		static ComponentEventId EVENT_ON_HEALTH_CHANGED;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		virtual void Initialize() override;

		const util::PUInt16Property &GetHealthProperty() const;
		const util::PUInt16Property &GetMaxHealthProperty() const;
		uint16_t GetHealth() const;
		uint16_t GetMaxHealth() const;
		virtual void SetHealth(uint16_t health);
		virtual void SetMaxHealth(uint16_t maxHealth);

		virtual void Save(udm::LinkedPropertyWrapper &udm) override;
		virtual void Load(udm::LinkedPropertyWrapper &udm,uint32_t version) override;

		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
	protected:
		BaseHealthComponent(BaseEntity &ent);
		virtual void OnTakeDamage(DamageInfo &info);

		util::PUInt16Property m_health;
		util::PUInt16Property m_maxHealth;
	};
	struct DLLNETWORK CEOnTakenDamage
		: public ComponentEvent
	{
		CEOnTakenDamage(DamageInfo &damageInfo,uint16_t oldHealth,uint16_t newHealth);
		virtual void PushArguments(lua_State *l) override;
		DamageInfo &damageInfo;
		uint16_t oldHealth;
		uint16_t newHealth;
	};
	struct DLLNETWORK CEOnHealthChanged
		: public ComponentEvent
	{
		CEOnHealthChanged(uint16_t oldHealth,uint16_t newHealth);
		virtual void PushArguments(lua_State *l) override;
		uint16_t oldHealth;
		uint16_t newHealth;
	};
};

#endif
