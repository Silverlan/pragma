#ifndef __S_POINT_CONSTRAINT_DOF_H__
#define __S_POINT_CONSTRAINT_DOF_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/constraints/point_constraint_dof.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SPointConstraintDoFComponent final
		: public BasePointConstraintDoFComponent,
		public SBaseNetComponent
	{
	public:
		using BasePointConstraintDoFComponent::BasePointConstraintDoFComponent;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER PointConstraintDoF
	: public SBaseEntity
{
protected:

public:
	virtual void Initialize() override;
};

#endif