#ifndef __S_ENV_DECAL_H__
#define __S_ENV_DECAL_H__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/effects/s_env_sprite.h"

namespace pragma
{
	class DLLSERVER SDecalComponent final
		: public BaseEntityComponent
	{
	public:
		SDecalComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvDecal
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
