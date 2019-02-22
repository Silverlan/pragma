#ifndef __C_ENV_LIGHT_SPOT_VOL_H__
#define __C_ENV_LIGHT_SPOT_VOL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"

#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/environment/lights/env_light_spot_vol.h>

namespace pragma
{
	class DLLCLIENT CLightSpotVolComponent final
		: public BaseEnvLightSpotVolComponent,
		public CBaseNetComponent
	{
	public:
		CLightSpotVolComponent(BaseEntity &ent) : BaseEnvLightSpotVolComponent(ent) {}
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual void OnEntitySpawn() override;
	protected:
		void InitializeVolumetricLight();
	};
};

class DLLCLIENT CEnvLightSpotVol
	: public CBaseEntity
{
	virtual void Initialize() override;
};

#endif
