// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_LIGHT_SPOT_H__
#define __S_ENV_LIGHT_SPOT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/lights/s_env_light.h"
#include <pragma/entities/environment/lights/env_light_spot.h>

namespace pragma {
	class DLLSERVER SLightSpotComponent final : public BaseEnvLightSpotComponent, public SBaseNetComponent {
	  public:
		SLightSpotComponent(BaseEntity &ent) : BaseEnvLightSpotComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void SetOuterConeAngle(float ang) override;
		virtual void SetBlendFraction(float fraction) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void SetConeStartOffset(float offset) override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

class DLLSERVER EnvLightSpot : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
