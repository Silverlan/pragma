// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.client:entities.components.child;

export import :entities.components.entity;

export namespace pragma {
	class CParticleSystemComponent;
	class DLLCLIENT CChildComponent final : public BaseChildComponent, public CBaseNetComponent {
	  public:
		CChildComponent(pragma::ecs::BaseEntity &ent) : BaseChildComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void OnRemove() override;
	};
};
