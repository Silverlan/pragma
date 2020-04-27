/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_TOGGLE_COMPONENT_HPP__
#define __S_TOGGLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/basetoggle.h>

namespace pragma
{
	class DLLSERVER SToggleComponent final
		: public BaseToggleComponent,
		public SBaseNetComponent
	{
	public:
		SToggleComponent(BaseEntity &ent) : BaseToggleComponent(ent) {}
		virtual void TurnOn() override;
		virtual void TurnOff() override;

		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
