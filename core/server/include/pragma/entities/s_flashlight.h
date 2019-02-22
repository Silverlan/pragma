#ifndef __S_FLASHLIGHT_H__
#define __S_FLASHLIGHT_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/baseflashlight.h>

namespace pragma
{
	class DLLSERVER SFlashlightComponent final
		: public BaseFlashlightComponent
	{
	public:
		SFlashlightComponent(BaseEntity &ent) : BaseFlashlightComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER Flashlight
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
