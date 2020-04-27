/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_WHEEL_COMPONENT_HPP__
#define __S_WHEEL_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_wheel_component.hpp>

namespace pragma
{
	class DLLSERVER SWheelComponent final
		: public BaseWheelComponent,
		public SBaseSnapshotComponent
	{
	public:
		SWheelComponent(BaseEntity &ent) : BaseWheelComponent(ent) {}
		virtual ~SWheelComponent() override;
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual void SendSnapshotData(NetPacket &packet,pragma::BasePlayerComponent &pl) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual bool ShouldTransmitSnapshotData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};
class DLLSERVER SWheel
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif