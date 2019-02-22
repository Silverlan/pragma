#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/physenvironment.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/physics/physcontact.h"

extern DLLENGINE Engine *engine;

namespace Lua
{
	namespace PhysContact
	{
		static void GetEntity1(lua_State *l,::PhysContact &contact);
		static void GetEntity2(lua_State *l,::PhysContact &contact);
		static void GetHitNormal(lua_State *l,::PhysContact &contact);
		static void GetCollisionObject1(lua_State *l,::PhysContact &contact);
		static void GetCollisionObject2(lua_State *l,::PhysContact &contact);
		static void GetPhysicsObject1(lua_State *l,::PhysContact &contact);
		static void GetPhysicsObject2(lua_State *l,::PhysContact &contact);
		static void GetContactPos1(lua_State *l,::PhysContact &contact);
		static void GetContactPos2(lua_State *l,::PhysContact &contact);
	};
};

void Lua::PhysContact::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<::PhysContact>("Contact");
	classDef.def("GetEntity1",&GetEntity1);
	classDef.def("GetEntity2",&GetEntity2);
	classDef.def("GetHitNormal",&GetHitNormal);
	classDef.def("GetCollisionObject1",&GetCollisionObject1);
	classDef.def("GetCollisionObject2",&GetCollisionObject2);
	classDef.def("GetPhysicsObject1",&GetPhysicsObject1);
	classDef.def("GetPhysicsObject2",&GetPhysicsObject2);
	classDef.def("GetContactPos1",&GetContactPos1);
	classDef.def("GetContactPos2",&GetContactPos2);
	mod[classDef];
}

void Lua::PhysContact::GetEntity1(lua_State *l,::PhysContact &contact)
{
	if(contact.entA == nullptr)
		return;
	lua_pushentity(l,contact.entA);
}
void Lua::PhysContact::GetEntity2(lua_State *l,::PhysContact &contact)
{
	if(contact.entB == nullptr)
		return;
	lua_pushentity(l,contact.entB);
}
void Lua::PhysContact::GetHitNormal(lua_State *l,::PhysContact &contact)
{
	Lua::Push<Vector3>(l,contact.hitNormal);
}
void Lua::PhysContact::GetCollisionObject1(lua_State *l,::PhysContact &contact)
{
	if(contact.objA == nullptr)
		return;
	contact.objA->GetLuaObject()->push(l);
}
void Lua::PhysContact::GetCollisionObject2(lua_State *l,::PhysContact &contact)
{
	if(contact.objB == nullptr)
		return;
	contact.objB->GetLuaObject()->push(l);
}
void Lua::PhysContact::GetPhysicsObject1(lua_State *l,::PhysContact &contact)
{
	if(contact.physA == nullptr)
		return;
	luabind::object(l,contact.physA->GetHandle()).push(l);
}
void Lua::PhysContact::GetPhysicsObject2(lua_State *l,::PhysContact &contact)
{
	if(contact.physB == nullptr)
		return;
	luabind::object(l,contact.physB->GetHandle()).push(l);
}
void Lua::PhysContact::GetContactPos1(lua_State *l,::PhysContact &contact)
{
	Lua::Push<Vector3>(l,contact.posA);
}
void Lua::PhysContact::GetContactPos2(lua_State *l,::PhysContact &contact)
{
	Lua::Push<Vector3>(l,contact.posB);
}
