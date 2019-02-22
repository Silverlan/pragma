#ifndef __C_SKYBOX_H__
#define __C_SKYBOX_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseskybox.h>

namespace pragma
{
	class DLLCLIENT CSkyboxComponent final
		: public BaseSkyboxComponent
	{
	public:
		CSkyboxComponent(BaseEntity &ent) : BaseSkyboxComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CSkybox
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif