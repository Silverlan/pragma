/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_SURFACE_COMPONENT_HPP__
#define __S_SURFACE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_surface_component.hpp>

namespace pragma {
	class DLLSERVER SSurfaceComponent final : public BaseSurfaceComponent, public SBaseNetComponent {
	  public:
		SSurfaceComponent(BaseEntity &ent) : BaseSurfaceComponent(ent) {}
		virtual void SetPlane(const umath::Plane &plane) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
