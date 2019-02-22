#ifndef __C_TOGGLE_COMPONENT_HPP__
#define __C_TOGGLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/basetoggle.h>

namespace pragma
{
	class DLLCLIENT CToggleComponent final
		: public BaseToggleComponent,
		public CBaseNetComponent
	{
	public:
		CToggleComponent(BaseEntity &ent) : BaseToggleComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(UInt32 eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
