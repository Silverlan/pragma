/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_PROP_BASE_HPP__
#define __S_PROP_BASE_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/prop/prop_base.h>

namespace pragma {
	class DLLSERVER PropComponent final : public BasePropComponent, public SBaseNetComponent {
	  public:
		PropComponent(BaseEntity &ent) : BasePropComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};

class DLLSERVER SBaseProp : public SBaseEntity {
  protected:
};

#endif
