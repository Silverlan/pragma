#ifndef __S_ENV_SMOKE_TRAIL_H__
#define __S_ENV_SMOKE_TRAIL_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/effects/env_smoke_trail.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SSmokeTrailComponent final
		: public BaseEnvSmokeTrailComponent,
		public SBaseNetComponent
	{
	public:
		SSmokeTrailComponent(BaseEntity &ent) : BaseEnvSmokeTrailComponent(ent) {}
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSmokeTrail
	: public SBaseEntity
{
protected:
public:
	virtual void Initialize() override;
};

#endif
