// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_FOG_CONTROLLER_H__
#define __S_ENV_FOG_CONTROLLER_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/env_fog_controller.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma {
	class DLLSERVER SFogControllerComponent final : public BaseEnvFogControllerComponent, public SBaseNetComponent {
	  public:
		SFogControllerComponent(BaseEntity &ent) : BaseEnvFogControllerComponent(ent) {}
		virtual void SetFogStart(float start) override;
		virtual void SetFogEnd(float end) override;
		virtual void SetMaxDensity(float density) override;
		virtual void SetFogType(util::FogType type) override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvFogController : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
