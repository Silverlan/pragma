#ifndef __S_ENV_LIGHT_SPOT_VOL_H__
#define __S_ENV_LIGHT_SPOT_VOL_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/environment/lights/env_light_spot_vol.h>

namespace pragma
{
	class DLLSERVER SLightSpotVolComponent final
		: public BaseEnvLightSpotVolComponent,
		public SBaseNetComponent
	{
	public:
		SLightSpotVolComponent(BaseEntity &ent) : BaseEnvLightSpotVolComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvLightSpotVol
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
