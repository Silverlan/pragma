#ifndef __C_ENV_WEATHER_H__
#define __C_ENV_WEATHER_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_weather.h"

namespace pragma
{
	class DLLCLIENT CWeatherComponent final
		: public BaseEnvWeatherComponent
	{
	public:
		CWeatherComponent(BaseEntity &ent) : BaseEnvWeatherComponent(ent) {}
		virtual ~CWeatherComponent() override;
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	protected:
		EntityHandle m_hParticle = {};
	};
};

class EntityHandle;
class DLLCLIENT CEnvWeather
	: public CBaseEntity
{
public:
	virtual void Initialize() override;

};

#endif