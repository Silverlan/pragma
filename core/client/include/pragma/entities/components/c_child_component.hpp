/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_CHILD_COMPONENT_HPP__
#define __C_CHILD_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_child_component.hpp>

namespace pragma {
	class CParticleSystemComponent;
	class DLLCLIENT CChildComponent final : public BaseChildComponent, public CBaseNetComponent {
	  public:
		CChildComponent(BaseEntity &ent) : BaseChildComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void OnRemove() override;
	};
};

#endif
