#ifndef __S_PROP_PHYSICS_H__
#define __S_PROP_PHYSICS_H__

#include "pragma/serverdefinitions.h"
#include <pragma/entities/prop/prop_physics.hpp>

namespace pragma
{
	class DLLSERVER SPropPhysicsComponent final
		: public BasePropPhysicsComponent
	{
	public:
		SPropPhysicsComponent(BaseEntity &ent) : BasePropPhysicsComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER PropPhysics
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif