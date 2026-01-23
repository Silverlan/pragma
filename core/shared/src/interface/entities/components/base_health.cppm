// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_health;

export import :entities.components.base;
export import :game.damage_info;

export namespace pragma {
	namespace baseHealthComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_TAKEN_DAMAGE)
		REGISTER_COMPONENT_EVENT(EVENT_ON_HEALTH_CHANGED)
	}
	class DLLNETWORK BaseHealthComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		static void RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember);
		virtual void Initialize() override;

		const util::PUInt16Property &GetHealthProperty() const;
		const util::PUInt16Property &GetMaxHealthProperty() const;
		uint16_t GetHealth() const;
		uint16_t GetMaxHealth() const;
		virtual void SetHealth(uint16_t health);
		virtual void SetMaxHealth(uint16_t maxHealth);

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
	  protected:
		BaseHealthComponent(ecs::BaseEntity &ent);
		virtual void OnTakeDamage(game::DamageInfo &info);

		util::PUInt16Property m_health;
		util::PUInt16Property m_maxHealth;
	};
	struct DLLNETWORK CEOnTakenDamage : public ComponentEvent {
		CEOnTakenDamage(game::DamageInfo &damageInfo, uint16_t oldHealth, uint16_t newHealth);
		virtual void PushArguments(lua::State *l) override;
		game::DamageInfo &damageInfo;
		uint16_t oldHealth;
		uint16_t newHealth;
	};
	struct DLLNETWORK CEOnHealthChanged : public ComponentEvent {
		CEOnHealthChanged(uint16_t oldHealth, uint16_t newHealth);
		virtual void PushArguments(lua::State *l) override;
		uint16_t oldHealth;
		uint16_t newHealth;
	};
};
