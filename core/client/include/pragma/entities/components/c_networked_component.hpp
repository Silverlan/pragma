#ifndef __C_NETWORKED_COMPONENT_HPP__
#define __C_NETWORKED_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_networked_component.hpp>

namespace pragma
{
	class DLLCLIENT CNetworkedComponent final
		: public BaseNetworkedComponent,
		public CBaseNetComponent
	{
	public:
		CNetworkedComponent(BaseEntity &ent) : BaseNetworkedComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

#endif
