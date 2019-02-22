#ifndef __S_SKYBOX_H__
#define __S_SKYBOX_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/baseskybox.h>

namespace pragma
{
	class DLLSERVER SSkyboxComponent final
		: public BaseSkyboxComponent
	{
	public:
		SSkyboxComponent(BaseEntity &ent) : BaseSkyboxComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER Skybox
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif