/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
