/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_POINT_AT_TARGET_COMPONENT_HPP__
#define __S_POINT_AT_TARGET_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_point_at_target_component.hpp>
#include <pragma/lua/luaapi.h>

namespace pragma {
	class DLLSERVER SPointAtTargetComponent final : public BasePointAtTargetComponent, public SBaseNetComponent {
	  public:
		SPointAtTargetComponent(BaseEntity &ent) : BasePointAtTargetComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;

		using BasePointAtTargetComponent::SetPointAtTarget;
	  protected:
		virtual void SetPointAtTarget(BaseEntity *ent) override;
	};
};

#endif
