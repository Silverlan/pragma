// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_POINT_AT_TARGET_COMPONENT_HPP__
#define __C_POINT_AT_TARGET_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_point_at_target_component.hpp>
#include <pragma/lua/luaapi.h>

namespace pragma {
	class DLLCLIENT CPointAtTargetComponent final : public BasePointAtTargetComponent, public CBaseNetComponent {
	  public:
		CPointAtTargetComponent(BaseEntity &ent) : BasePointAtTargetComponent(ent) {}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		using BasePointAtTargetComponent::SetPointAtTarget;
	  protected:
		virtual bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
	};
};

#endif
