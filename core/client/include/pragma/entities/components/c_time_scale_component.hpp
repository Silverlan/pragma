// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_TIME_SCALE_COMPONENT_HPP__
#define __C_TIME_SCALE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_time_scale_component.hpp>

namespace pragma {
	class DLLCLIENT CTimeScaleComponent final : public BaseTimeScaleComponent, public CBaseNetComponent {
	  public:
		CTimeScaleComponent(BaseEntity &ent) : BaseTimeScaleComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void ReceiveData(NetPacket &packet) override {}
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
	};
};

#endif
