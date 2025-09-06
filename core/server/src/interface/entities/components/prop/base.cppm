// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include <pragma/entities/prop/prop_base.h>

export module pragma.server.entities.components.prop.base;

import pragma.server.entities;
import pragma.server.entities.components.entity;

export {
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
};
