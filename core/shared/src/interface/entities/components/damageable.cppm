// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.damageable;

export import :entities.components.base;
export import :game.damage_info;

export namespace pragma {
	namespace damageableComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_TAKE_DAMAGE)
	}
	class DLLNETWORK DamageableComponent final : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		DamageableComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;

		// Called right before the entity is about to take damage
		virtual void OnTakeDamage(game::DamageInfo &info);

		virtual void TakeDamage(game::DamageInfo &info);
		virtual void InitializeLuaObject(lua::State *l) override;
	};
	struct DLLNETWORK CEOnTakeDamage : public ComponentEvent {
		CEOnTakeDamage(game::DamageInfo &damageInfo);
		virtual void PushArguments(lua::State *l) override;
		game::DamageInfo &damageInfo;
	};
};
