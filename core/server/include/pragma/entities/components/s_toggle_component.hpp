// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_TOGGLE_COMPONENT_HPP__
#define __S_TOGGLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/basetoggle.h>

namespace pragma {
	class DLLSERVER SToggleComponent final : public BaseToggleComponent, public SBaseNetComponent {
	  public:
		SToggleComponent(BaseEntity &ent) : BaseToggleComponent(ent) {}
		virtual void TurnOn() override;
		virtual void TurnOff() override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
