// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:entities.components.base_flammable;

export import :entities.components.base;

export namespace pragma {
	namespace baseFlammableComponent {
		REGISTER_COMPONENT_EVENT(EVENT_ON_IGNITED)
		REGISTER_COMPONENT_EVENT(EVENT_ON_EXTINGUISHED)
	}
	class DLLNETWORK BaseFlammableComponent : public BaseEntityComponent {
	  public:
		static void RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);
		virtual ~BaseFlammableComponent() override;
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;

		const util::PBoolProperty &GetOnFireProperty() const;
		const util::PBoolProperty &GetIgnitableProperty() const;
		bool IsOnFire() const;
		bool IsIgnitable() const;
		virtual util::EventReply Ignite(float duration, ecs::BaseEntity *attacker = nullptr, ecs::BaseEntity *inflictor = nullptr);
		virtual void Extinguish();
		virtual void SetIgnitable(bool b);
		virtual void OnTick(double dt) override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseFlammableComponent(ecs::BaseEntity &ent);
		NetEventId m_netEvIgnite = INVALID_NET_EVENT;
		NetEventId m_netEvExtinguish = INVALID_NET_EVENT;
		NetEventId m_netEvSetIgnitable = INVALID_NET_EVENT;
		util::PBoolProperty m_bIsOnFire;
		util::PBoolProperty m_bIgnitable;
		float m_tExtinguishTime = 0.f;
	};
	struct DLLNETWORK CEOnIgnited : public ComponentEvent {
		CEOnIgnited(float duration, ecs::BaseEntity *attacker, ecs::BaseEntity *inflictor);
		virtual void PushArguments(lua::State *l) override;
		float duration;
		EntityHandle attacker;
		EntityHandle inflictor;
	};
};
