#ifndef __C_NAME_COMPONENT_HPP__
#define __C_NAME_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_name_component.hpp>

namespace pragma
{
	class DLLCLIENT CNameComponent final
		: public BaseNameComponent,
		public CBaseNetComponent
	{
	public:
		CNameComponent(BaseEntity &ent) : BaseNameComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
