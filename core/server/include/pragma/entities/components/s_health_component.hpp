// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_HEALTH_COMPONENT_HPP__
#define __S_HEALTH_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_health_component.hpp>

namespace pragma {
	class DLLSERVER SHealthComponent final : public BaseHealthComponent, public SBaseNetComponent {
	  public:
		SHealthComponent(BaseEntity &ent) : BaseHealthComponent(ent) {}
		virtual void SetHealth(unsigned short health) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
