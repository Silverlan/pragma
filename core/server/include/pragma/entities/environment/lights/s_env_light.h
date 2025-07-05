// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_LIGHT_H__
#define __S_ENV_LIGHT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma {
	class DLLSERVER SLightComponent final : public BaseEnvLightComponent, public SBaseNetComponent {
	  public:
		SLightComponent(BaseEntity &ent) : BaseEnvLightComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void SetShadowType(ShadowType type) override;
		virtual void InitializeLuaObject(lua_State *l) override;

		virtual void SetFalloffExponent(float falloffExponent) override;
	};
};

#endif
