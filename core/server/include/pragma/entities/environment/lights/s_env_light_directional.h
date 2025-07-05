// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_LIGHT_DIRECTIONAL_H__
#define __S_ENV_LIGHT_DIRECTIONAL_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/lights/s_env_light.h"
#include "pragma/entities/environment/lights/env_light_directional.h"

namespace pragma {
	class DLLSERVER SLightDirectionalComponent final : public BaseEnvLightDirectionalComponent, public SBaseNetComponent {
	  public:
		SLightDirectionalComponent(BaseEntity &ent) : BaseEnvLightDirectionalComponent(ent) {};
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void SetAmbientColor(const Color &color) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

class DLLSERVER EnvLightDirectional : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
