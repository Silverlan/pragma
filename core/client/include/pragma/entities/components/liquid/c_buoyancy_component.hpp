// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_BUOYANCY_COMPONENT_HPP__
#define __C_BUOYANCY_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/liquid/base_buoyancy_component.hpp>

namespace pragma {
	class DLLCLIENT CBuoyancyComponent final : public BaseBuoyancyComponent, public CBaseNetComponent {
	  public:
		CBuoyancyComponent(BaseEntity &ent) : BaseBuoyancyComponent(ent) {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
