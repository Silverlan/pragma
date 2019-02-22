#ifndef __C_TRIGGER_TOUCH_H__
#define __C_TRIGGER_TOUCH_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include <pragma/entities/trigger/base_trigger_touch.hpp>

namespace pragma
{
	class DLLCLIENT CTouchComponent final
		: public BaseTouchComponent
	{
	public:
		CTouchComponent(BaseEntity &ent) : BaseTouchComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	};
};

class DLLCLIENT CTriggerTouch
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif