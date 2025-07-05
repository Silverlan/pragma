// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_FIELD_ANGLE_COMPONENT_HPP__
#define __S_FIELD_ANGLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_field_angle_component.hpp>

namespace pragma {
	class DLLSERVER SFieldAngleComponent final : public BaseFieldAngleComponent, public SBaseNetComponent {
	  public:
		SFieldAngleComponent(BaseEntity &ent) : BaseFieldAngleComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
