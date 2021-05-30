/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_WHEEL_HPP__
#define __C_WHEEL_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_wheel_component.hpp>

namespace pragma
{
	class DLLCLIENT CWheelComponent final
		: public BaseWheelComponent,
		public CBaseSnapshotComponent
	{
	public:
		CWheelComponent(BaseEntity &ent) : BaseWheelComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual bool ShouldTransmitSnapshotData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

class DLLCLIENT CWheel
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
