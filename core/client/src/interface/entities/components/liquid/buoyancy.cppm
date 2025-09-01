// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/liquid/base_buoyancy_component.hpp>

export module pragma.client.entities.components:liquid_buoyancy;

export namespace pragma {
	class DLLCLIENT CBuoyancyComponent final : public BaseBuoyancyComponent, public CBaseNetComponent {
	  public:
		CBuoyancyComponent(BaseEntity &ent) : BaseBuoyancyComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};
