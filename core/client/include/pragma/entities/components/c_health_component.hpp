#ifndef __C_HEALTH_COMPONENT_HPP__
#define __C_HEALTH_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_health_component.hpp>

namespace pragma
{
	class DLLCLIENT CHealthComponent final
		: public BaseHealthComponent,
		public CBaseNetComponent
	{
	public:
		CHealthComponent(BaseEntity &ent) : BaseHealthComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
