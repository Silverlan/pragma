#ifndef __S_FUNCBUTTON_H__
#define __S_FUNCBUTTON_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/func/basefuncbutton.h"

namespace pragma
{
	class DLLSERVER SButtonComponent final
		: public BaseFuncButtonComponent
	{
	public:
		SButtonComponent(BaseEntity &ent) : BaseFuncButtonComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER FuncButton
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
