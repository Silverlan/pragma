// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.radius;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CRadiusComponent final : public BaseRadiusComponent, public CBaseNetComponent {
	  public:
		CRadiusComponent(pragma::ecs::BaseEntity &ent);

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override;
	};
};
