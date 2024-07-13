/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_OBSERVABLE_COMPONENT_HPP__
#define __S_OBSERVABLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_observable_component.hpp>

namespace pragma {
	class DLLSERVER SObservableComponent final : public BaseObservableComponent, public SBaseNetComponent {
	  public:
		SObservableComponent(BaseEntity &ent) : BaseObservableComponent(ent) {}

		virtual void SetLocalCameraOrigin(CameraType type, const Vector3 &origin) override;
		virtual void SetLocalCameraOffset(CameraType type, const Vector3 &offset) override;
		virtual void SetViewOffset(const Vector3 &offset) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

#endif
