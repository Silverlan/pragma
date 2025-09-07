// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_radius_component.hpp>

export module pragma.client.entities.components.radius;

export namespace pragma {
	class DLLCLIENT CRadiusComponent final : public BaseRadiusComponent, public CBaseNetComponent {
	  public:
		CRadiusComponent(BaseEntity &ent);

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override;
	};
};
