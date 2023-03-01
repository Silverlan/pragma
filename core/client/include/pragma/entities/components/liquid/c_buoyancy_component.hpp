/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
