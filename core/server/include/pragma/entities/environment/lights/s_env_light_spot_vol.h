// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_LIGHT_SPOT_VOL_H__
#define __S_ENV_LIGHT_SPOT_VOL_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/environment/lights/env_light_spot_vol.h>

namespace pragma {
	class DLLSERVER SLightSpotVolComponent final : public BaseEnvLightSpotVolComponent, public SBaseNetComponent {
	  public:
		SLightSpotVolComponent(BaseEntity &ent) : BaseEnvLightSpotVolComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void SetSpotlightTarget(BaseEntity &ent) override;
	};
};

class DLLSERVER EnvLightSpotVol : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
