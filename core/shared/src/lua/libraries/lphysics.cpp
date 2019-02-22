#include "stdafx_shared.h"
#include <pragma/engine.h>
#include "pragma/networkstate/networkstate.h"
#include <pragma/game/game.h>
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/physcollisionobject.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/physics/physshape.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"

extern DLLENGINE Engine *engine;

void Lua::PushConstraint(lua_State *l,const ConstraintHandle &hConstraint)
{
	PushConstraint(l,hConstraint.get());
}
void Lua::PushConstraint(lua_State *l,::PhysConstraint *constraint)
{
	if(constraint == nullptr)
	{
		PushNil(l);
		return;
	}
	constraint->GetLuaObject()->push(l);
}
