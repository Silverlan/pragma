#ifndef __S_COLOR_COMPONENT_HPP__
#define __S_COLOR_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_color_component.hpp>

namespace pragma
{
	class DLLSERVER SColorComponent final
		: public BaseColorComponent,
		public SBaseNetComponent
	{
	public:
		SColorComponent(BaseEntity &ent) : BaseColorComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;

		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
