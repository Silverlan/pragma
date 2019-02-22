#ifndef __C_ENV_WIND_HPP__
#define __C_ENV_WIND_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_wind.hpp"
#include "pragma/entities/components/c_entity_component.hpp"

namespace pragma
{
	class DLLCLIENT CWindComponent final
		: public BaseEnvWindComponent,
		public CBaseNetComponent
	{
	public:
		CWindComponent(BaseEntity &ent) : BaseEnvWindComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

class EntityHandle;
class DLLCLIENT CEnvWind
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
