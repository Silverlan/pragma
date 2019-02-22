#ifndef __S_TRIGGER_REMOVE_H__
#define __S_TRIGGER_REMOVE_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/trigger/base_trigger_remove.h>

namespace pragma
{
	class DLLSERVER STriggerRemoveComponent final
		: public BaseTriggerRemoveComponent
	{
	public:
		STriggerRemoveComponent(BaseEntity &ent) : BaseTriggerRemoveComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER TriggerRemove
	: public SBaseEntity
{
protected:
public:
	virtual void Initialize() override;
};

#endif