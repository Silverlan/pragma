#ifndef __C_OWNABLE_COMPONENT_HPP__
#define __C_OWNABLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_ownable_component.hpp>
#include <pragma/lua/luaapi.h>

namespace pragma
{
	class DLLCLIENT COwnableComponent final
		: public BaseOwnableComponent,
		public CBaseNetComponent
	{
	public:
		COwnableComponent(BaseEntity &ent) : BaseOwnableComponent(ent) {}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		virtual bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
	};
};

#endif
