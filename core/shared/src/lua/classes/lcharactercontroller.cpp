#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/physkinematiccharactercontroller.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"

extern DLLENGINE Engine *engine;

namespace Lua
{
	namespace PhysKinematicCharacterController
	{
		static void SetOrigin(lua_State *l,::PhysKinematicCharacterController &controller,Vector3 &origin);
		static void GetOrigin(lua_State *l,::PhysKinematicCharacterController &controller);
	};
};

void Lua::PhysKinematicCharacterController::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<::PhysKinematicCharacterController>("CharacterController");
	classDef.def("SetOrigin",&SetOrigin);
	classDef.def("GetOrigin",&GetOrigin);
	mod[classDef];
}
void Lua::PhysKinematicCharacterController::SetOrigin(lua_State*,::PhysKinematicCharacterController &controller,Vector3 &origin)
{
	btPairCachingGhostObject *ghost = controller.getGhostObject();
	btTransform &t = ghost->getWorldTransform();
	btVector3 &p = t.getOrigin();
	p.setX(origin.x);
	p.setY(origin.y);
	p.setZ(origin.z);
}

void Lua::PhysKinematicCharacterController::GetOrigin(lua_State *l,::PhysKinematicCharacterController &controller)
{
	btPairCachingGhostObject *ghost = controller.getGhostObject();
	btTransform &t = ghost->getWorldTransform();
	btVector3 &p = t.getOrigin();
	Lua::Push<Vector3>(l,Vector3(p.x(),p.y(),p.z()));
}
