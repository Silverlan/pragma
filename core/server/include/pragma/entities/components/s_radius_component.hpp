#ifndef __S_RADIUS_COMPONENT_HPP__
#define __S_RADIUS_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_radius_component.hpp>

namespace pragma
{
	class DLLSERVER SRadiusComponent final
		: public BaseRadiusComponent,
		public SBaseNetComponent
	{
	public:
		SRadiusComponent(BaseEntity &ent) : BaseRadiusComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
