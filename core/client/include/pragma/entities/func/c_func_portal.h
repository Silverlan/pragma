#ifndef __C_FUNC_PORTAL_H__
#define __C_FUNC_PORTAL_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/func/basefuncportal.h"

namespace pragma
{
	class DLLCLIENT CFuncPortalComponent final
		: public BaseFuncPortalComponent
	{
	public:
		CFuncPortalComponent(BaseEntity &ent) : BaseFuncPortalComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CFuncPortal
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif