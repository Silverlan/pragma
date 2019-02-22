#ifndef __S_POINT_TARGET_H__
#define __S_POINT_TARGET_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/point_target.h"

namespace pragma
{
	class DLLSERVER SPointTargetComponent final
		: public BasePointTargetComponent
	{
	public:
		SPointTargetComponent(BaseEntity &ent) : BasePointTargetComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER PointTarget
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif