// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_weather.h"

export module pragma.client.entities.components.env_weather;

export {
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
};
