/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_BUOYANCY_COMPONENT_HPP__
#define __S_BUOYANCY_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/liquid/base_buoyancy_component.hpp>

namespace pragma {
	class DLLSERVER SBuoyancyComponent final : public BaseBuoyancyComponent, public SBaseNetComponent {
	  public:
		SBuoyancyComponent(BaseEntity &ent) : BaseBuoyancyComponent(ent) {}
		virtual void Initialize() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
