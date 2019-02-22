#ifndef __C_RADIUS_COMPONENT_HPP__
#define __C_RADIUS_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_radius_component.hpp>

namespace pragma
{
	class DLLCLIENT CRadiusComponent final
		: public BaseRadiusComponent,
		public CBaseNetComponent
	{
	public:
		CRadiusComponent(BaseEntity &ent) : BaseRadiusComponent(ent) {}

		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
