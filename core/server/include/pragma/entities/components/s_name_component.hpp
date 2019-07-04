#ifndef __S_NAME_COMPONENT_HPP__
#define __S_NAME_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_name_component.hpp>

namespace pragma
{
	class DLLSERVER SNameComponent final
		: public BaseNameComponent,
		public SBaseNetComponent
	{
	public:
		SNameComponent(BaseEntity &ent) : BaseNameComponent(ent) {}
		virtual void SetName(std::string name) override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
