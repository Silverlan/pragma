/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/environment.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/physics/contact.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/physics/collision_object.hpp"

extern DLLNETWORK Engine *engine;

void Lua::PhysContact::register_class(lua_State *l, luabind::module_ &mod)
{
	auto classDefPoint = luabind::class_<::pragma::physics::ContactPoint>("ContactPoint");
	classDefPoint.def_readwrite("impulse", &::pragma::physics::ContactPoint::impulse);
	classDefPoint.def_readwrite("normal", &::pragma::physics::ContactPoint::normal);
	classDefPoint.def_readwrite("position", &::pragma::physics::ContactPoint::position);
	classDefPoint.def_readwrite("distance", &::pragma::physics::ContactPoint::distance);
	classDefPoint.property("material0", static_cast<luabind::optional<pragma::physics::IMaterial> (*)(lua_State *, ::pragma::physics::ContactPoint &)>([](lua_State *l, ::pragma::physics::ContactPoint &cp) -> luabind::optional<pragma::physics::IMaterial> {
		if(cp.material0.expired())
			return nil;
		return cp.material0->GetLuaObject(l);
	}));
	classDefPoint.property("material1", static_cast<luabind::optional<pragma::physics::IMaterial> (*)(lua_State *, ::pragma::physics::ContactPoint &)>([](lua_State *l, ::pragma::physics::ContactPoint &cp) -> luabind::optional<pragma::physics::IMaterial> {
		if(cp.material1.expired())
			return nil;
		return cp.material1->GetLuaObject(l);
	}));
	mod[classDefPoint];

	auto classDef = luabind::class_<::pragma::physics::ContactInfo>("ContactInfo");
	classDef.property("GetContactPointCount", static_cast<uint32_t (*)(lua_State *, ::pragma::physics::ContactInfo &)>([](lua_State *l, ::pragma::physics::ContactInfo &contactInfo) -> uint32_t { return contactInfo.contactPoints.size(); }));
	classDef.property("GetContactPoint", static_cast<pragma::physics::ContactPoint (*)(lua_State *, ::pragma::physics::ContactInfo &, uint32_t)>([](lua_State *l, ::pragma::physics::ContactInfo &contactInfo, uint32_t index) { return contactInfo.contactPoints.at(index); }));
	classDef.property("GetContactPoints",
	  static_cast<luabind::tableT<pragma::physics::ContactPoint> (*)(lua_State *, ::pragma::physics::ContactInfo &, uint32_t)>([](lua_State *l, ::pragma::physics::ContactInfo &contactInfo, uint32_t index) -> luabind::tableT<pragma::physics::ContactPoint> {
		  auto t = luabind::newtable(l);
		  auto idx = 1;
		  for(auto &p : contactInfo.contactPoints)
			  t[idx++] = p;
		  return t;
	  }));
	classDef.property("shape0", static_cast<luabind::optional<pragma::physics::IShape> (*)(lua_State *, ::pragma::physics::ContactInfo &)>([](lua_State *l, ::pragma::physics::ContactInfo &contactInfo) -> luabind::optional<pragma::physics::IShape> {
		if(contactInfo.shape0.expired())
			return nil;
		return contactInfo.shape0->GetLuaObject(l);
	}));
	classDef.property("shape1", static_cast<luabind::optional<pragma::physics::IShape> (*)(lua_State *, ::pragma::physics::ContactInfo &)>([](lua_State *l, ::pragma::physics::ContactInfo &contactInfo) -> luabind::optional<pragma::physics::IShape> {
		if(contactInfo.shape1.expired())
			return nil;
		return contactInfo.shape1->GetLuaObject(l);
	}));
	classDef.property("collisionObj0", static_cast<luabind::optional<pragma::physics::ICollisionObject> (*)(lua_State *, ::pragma::physics::ContactInfo &)>([](lua_State *l, ::pragma::physics::ContactInfo &contactInfo) -> luabind::optional<pragma::physics::ICollisionObject> {
		if(contactInfo.collisionObj0.IsExpired())
			return nil;
		return contactInfo.collisionObj0->GetLuaObject(l);
	}));
	classDef.property("collisionObj1", static_cast<luabind::optional<pragma::physics::ICollisionObject> (*)(lua_State *, ::pragma::physics::ContactInfo &)>([](lua_State *l, ::pragma::physics::ContactInfo &contactInfo) -> luabind::optional<pragma::physics::ICollisionObject> {
		if(contactInfo.collisionObj1.IsExpired())
			return nil;
		return contactInfo.collisionObj1->GetLuaObject(l);
	}));
	classDef.def_readwrite("flags", reinterpret_cast<std::underlying_type_t<decltype(::pragma::physics::ContactInfo::flags)> pragma::physics::ContactInfo::*>(&::pragma::physics::ContactInfo::flags));
	mod[classDef];
}
