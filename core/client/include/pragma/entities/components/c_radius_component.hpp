/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_RADIUS_COMPONENT_HPP__
#define __C_RADIUS_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_radius_component.hpp>

namespace pragma
{
	class DLLCLIENT CRadiusComponent final
		: public BaseRadiusComponent,
		public CBaseNetComponent
	{
	public:
		CRadiusComponent(BaseEntity &ent) : BaseRadiusComponent(ent) {}

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
