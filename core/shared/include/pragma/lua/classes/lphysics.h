#ifndef __LPHYSICS_H__
#define __LPHYSICS_H__
#include "pragma/networkdefinitions.h"
#include <pragma/definitions.h>
#include <pragma/physics/physapi.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <pragma/lua/luaapi.h>
#include <mathutil/glmutil.h>
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/constraint.hpp"

namespace Lua
{
	namespace physenv
	{
		DLLNETWORK void register_library(Lua::Interface &lua);
	};
	namespace PhysShape
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
	namespace PhysCollisionObj
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
	namespace PhysKinematicCharacterController
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
	namespace PhysContact
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
};

namespace Lua
{
	namespace PhysConstraint
	{
		DLLNETWORK void register_class(lua_State *l,luabind::module_ &mod);
	};
};

#endif
