// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_FIELD_ANGLE_COMPONENT_HPP__
#define __C_FIELD_ANGLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_field_angle_component.hpp>

namespace pragma {
	class DLLCLIENT CFieldAngleComponent final : public BaseFieldAngleComponent, public CBaseNetComponent {
	  public:
		CFieldAngleComponent(BaseEntity &ent) : BaseFieldAngleComponent(ent) {}

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};

#endif
