#ifndef __S_TRIGGER_PUSH_H__
#define __S_TRIGGER_PUSH_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/trigger/base_trigger_push.hpp>

namespace pragma
{
	class DLLSERVER STriggerPushComponent final
		: public BaseTriggerPushComponent
	{
	public:
		STriggerPushComponent(BaseEntity &ent) : BaseTriggerPushComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER TriggerPush
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif