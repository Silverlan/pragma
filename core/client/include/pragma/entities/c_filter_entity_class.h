#ifndef __C_FILTER_ENTITY_CLASS_H__
#define __C_FILTER_ENTITY_CLASS_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/basefilterentity.h>

namespace pragma
{
	class DLLCLIENT CFilterClassComponent final
		: public BaseFilterClassComponent
	{
	public:
		CFilterClassComponent(BaseEntity &ent) : BaseFilterClassComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CFilterEntityClass
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif