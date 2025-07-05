// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
