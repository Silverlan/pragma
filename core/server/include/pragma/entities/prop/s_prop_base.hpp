// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
