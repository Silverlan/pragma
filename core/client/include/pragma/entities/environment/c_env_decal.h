#ifndef __C_ENV_DECAL_H__
#define __C_ENV_DECAL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/environment/effects/c_env_sprite.h"

namespace pragma
{
	class DLLCLIENT CDecalComponent final
		: public BaseEntityComponent
	{
	public:
		CDecalComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
		virtual void Initialize() override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	protected:
		void ApplyDecal();
	};
};

class DLLCLIENT CEnvDecal
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif
