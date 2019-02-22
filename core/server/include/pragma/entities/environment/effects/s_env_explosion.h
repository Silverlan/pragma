#ifndef __S_ENV_EXPLOSION_H__
#define __S_ENV_EXPLOSION_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/effects/env_explosion.h"

namespace pragma
{
	class DLLSERVER SExplosionComponent final
		: public BaseEnvExplosionComponent
	{
	public:
		SExplosionComponent(BaseEntity &ent) : BaseEnvExplosionComponent(ent) {}
		virtual void Explode() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};
class DLLSERVER EnvExplosion
	: public SBaseEntity
{
protected:
public:
	virtual void Initialize() override;
};

#endif