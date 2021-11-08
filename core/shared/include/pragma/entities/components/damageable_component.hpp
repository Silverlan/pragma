/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __DAMAGEABLE_HPP__
#define __DAMAGEABLE_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

class DamageInfo;
namespace pragma
{
	class DLLNETWORK DamageableComponent final
		: public BaseEntityComponent
	{
	public:
		static ComponentEventId EVENT_ON_TAKE_DAMAGE;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent);
		DamageableComponent(BaseEntity &ent);
		virtual void Initialize() override;

		// Called right before the entity is about to take damage
		virtual void OnTakeDamage(DamageInfo &info);

		virtual void TakeDamage(DamageInfo &info);
		virtual void InitializeLuaObject(lua_State *l) override;
	};
	struct DLLNETWORK CEOnTakeDamage
		: public ComponentEvent
	{
		CEOnTakeDamage(DamageInfo &damageInfo);
		virtual void PushArguments(lua_State *l) override;
		DamageInfo &damageInfo;
	};
};

#endif
