#ifndef __C_ENV_EXPLOSION_H__
#define __C_ENV_EXPLOSION_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/effects/env_explosion.h"

namespace pragma
{
	class DLLCLIENT CExplosionComponent final
		: public BaseEnvExplosionComponent
	{
	public:
		CExplosionComponent(BaseEntity &ent) : BaseEnvExplosionComponent(ent) {}
		virtual void Initialize() override;
		virtual void Explode() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CEnvExplosion
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif