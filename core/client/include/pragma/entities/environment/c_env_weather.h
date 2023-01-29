/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_WEATHER_H__
#define __C_ENV_WEATHER_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_weather.h"

namespace pragma {
	class DLLCLIENT CWeatherComponent final : public BaseEnvWeatherComponent {
	  public:
		CWeatherComponent(BaseEntity &ent) : BaseEnvWeatherComponent(ent) {}
		virtual ~CWeatherComponent() override;
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	  protected:
		EntityHandle m_hParticle = {};
	};
};

class DLLCLIENT CEnvWeather : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
