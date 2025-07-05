// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_OBSERVABLE_COMPONENT_HPP__
#define __C_OBSERVABLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_observable_component.hpp>

namespace pragma {
	class DLLCLIENT CObservableComponent final : public BaseObservableComponent, public CBaseNetComponent {
	  public:
		CObservableComponent(BaseEntity &ent);
		virtual void SetLocalCameraOrigin(CameraType type, const Vector3 &offset) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};

#endif
