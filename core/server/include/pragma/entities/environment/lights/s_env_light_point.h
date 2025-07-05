// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_LIGHT_POINT_H__
#define __S_ENV_LIGHT_POINT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/lights/s_env_light.h"
#include "pragma/entities/environment/lights/env_light_point.h"

namespace pragma {
	class DLLSERVER SLightPointComponent final : public BaseEnvLightPointComponent {
	  public:
		SLightPointComponent(BaseEntity &ent) : BaseEnvLightPointComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

class DLLSERVER EnvLightPoint : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
