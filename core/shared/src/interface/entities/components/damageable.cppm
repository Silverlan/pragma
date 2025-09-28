// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include <sharedutils/property/util_property.hpp>

export module pragma.shared:entities.components.damageable;

export import :entities.components.base;
export import :game.damage_info;

export namespace pragma {
	class DLLNETWORK DamageableComponent final : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_TAKE_DAMAGE;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		DamageableComponent(BaseEntity &ent);
		virtual void Initialize() override;

		// Called right before the entity is about to take damage
		virtual void OnTakeDamage(DamageInfo &info);

		virtual void TakeDamage(DamageInfo &info);
		virtual void InitializeLuaObject(lua_State *l) override;
	};
	struct DLLNETWORK CEOnTakeDamage : public ComponentEvent {
		CEOnTakeDamage(DamageInfo &damageInfo);
		virtual void PushArguments(lua_State *l) override;
		DamageInfo &damageInfo;
	};
};
