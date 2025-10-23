// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:entities.components.base_flammable;

export import :entities.components.base;

export namespace pragma {
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
		virtual util::EventReply Ignite(float duration, pragma::ecs::BaseEntity *attacker = nullptr, pragma::ecs::BaseEntity *inflictor = nullptr);
		virtual void Extinguish();
		virtual void SetIgnitable(bool b);
		virtual void OnTick(double dt) override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
	  protected:
		BaseFlammableComponent(pragma::ecs::BaseEntity &ent);
		pragma::NetEventId m_netEvIgnite = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvExtinguish = pragma::INVALID_NET_EVENT;
		pragma::NetEventId m_netEvSetIgnitable = pragma::INVALID_NET_EVENT;
		util::PBoolProperty m_bIsOnFire;
		util::PBoolProperty m_bIgnitable;
		float m_tExtinguishTime = 0.f;
	};
	struct DLLNETWORK CEOnIgnited : public ComponentEvent {
		CEOnIgnited(float duration, pragma::ecs::BaseEntity *attacker, pragma::ecs::BaseEntity *inflictor);
		virtual void PushArguments(lua_State *l) override;
		float duration;
		EntityHandle attacker;
		EntityHandle inflictor;
	};
};
