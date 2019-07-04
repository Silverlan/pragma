#ifndef __S_POINT_CONSTRAINT_SLIDER_H__
#define __S_POINT_CONSTRAINT_SLIDER_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_slider.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SPointConstraintSliderComponent final
		: public BasePointConstraintSliderComponent,
		public SBaseNetComponent
	{
	public:
		SPointConstraintSliderComponent(BaseEntity &ent) : BasePointConstraintSliderComponent(ent) {}
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER PointConstraintSlider
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif