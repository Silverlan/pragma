/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/lphysobj.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_physobj.h"
#include <mathutil/glmutil.h>
#include "pragma/types.hpp"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/lua/policies/pair_policy.hpp"
#include "pragma/lua/policies/game_object_policy.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/physics/controller.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include <luabind/out_value_policy.hpp>

namespace Lua {
	namespace PhysObj {
		static bool IsValid(PhysObjHandle &hPhysObj);
		static luabind::tableT<pragma::physics::ICollisionObject> GetCollisionObjects(lua_State *l, ::PhysObj &physObj);
		static void SetCollisionFilterGroup(lua_State *l, ::PhysObj &physObj, int group);

		static bool IsOnGround(lua_State *l, ::PhysObj &physObj);
	};
};

static std::ostream &operator<<(std::ostream &out, const ::PhysObj *physObj)
{
	if(!physObj)
		out << "PhysObj[NULL]";
	else
		operator<<(out, *physObj);
	return out;
}

void Lua::PhysObj::register_class(lua_State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<::PhysObj>("Object");
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid", &IsValid);
	classDef.def("SetLinearVelocity", &::PhysObj::SetLinearVelocity);
	classDef.def("GetLinearVelocity", &::PhysObj::GetLinearVelocity);
	classDef.def("AddLinearVelocity", &::PhysObj::AddLinearVelocity);
	classDef.def("SetAngularVelocity", &::PhysObj::SetAngularVelocity);
	classDef.def("GetAngularVelocity", &::PhysObj::GetAngularVelocity);
	classDef.def("AddAngularVelocity", &::PhysObj::AddAngularVelocity);
	classDef.def("PutToSleep", &::PhysObj::PutToSleep);
	classDef.def("WakeUp", &::PhysObj::WakeUp);
	classDef.def("GetMass", &::PhysObj::GetMass);
	classDef.def("SetMass", &::PhysObj::SetMass);
	classDef.def("GetLinearFactor", &::PhysObj::GetLinearFactor);
	classDef.def("GetAngularFactor", &::PhysObj::GetAngularFactor);
	classDef.def("SetLinearFactor", &::PhysObj::SetLinearFactor);
	classDef.def("SetAngularFactor", &::PhysObj::SetAngularFactor);
	classDef.def("GetCollisionObjects", &GetCollisionObjects);
	classDef.def("GetOwner", &::PhysObj::GetOwner);
	classDef.def("SetCollisionFilterMask", &::PhysObj::SetCollisionFilterMask);
	classDef.def("GetCollisionFilterMask", &::PhysObj::GetCollisionFilterMask);
	classDef.def("SetCollisionFilterGroup", &SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup", static_cast<CollisionMask (::PhysObj::*)() const>(&::PhysObj::GetCollisionFilter));
	classDef.def("SetCollisionFilter", static_cast<void (::PhysObj::*)(CollisionMask, CollisionMask)>(&::PhysObj::SetCollisionFilter));
	classDef.def("SetDamping", &::PhysObj::SetDamping);
	classDef.def("SetLinearDamping", &::PhysObj::SetLinearDamping);
	classDef.def("SetAngularDamping", &::PhysObj::SetAngularDamping);
	classDef.def("GetLinearDamping", &::PhysObj::GetLinearDamping);
	classDef.def("GetAngularDamping", &::PhysObj::GetAngularDamping);
	classDef.def("ApplyForce", static_cast<void (::PhysObj::*)(const Vector3 &)>(&::PhysObj::ApplyForce));
	classDef.def("ApplyForce", static_cast<void (::PhysObj::*)(const Vector3 &, const Vector3 &)>(&::PhysObj::ApplyForce));
	classDef.def("ApplyImpulse", static_cast<void (::PhysObj::*)(const Vector3 &)>(&::PhysObj::ApplyImpulse));
	classDef.def("ApplyImpulse", static_cast<void (::PhysObj::*)(const Vector3 &, const Vector3 &)>(&::PhysObj::ApplyImpulse));
	classDef.def("ApplyTorque", &::PhysObj::ApplyTorque);
	classDef.def("ApplyTorqueImpulse", &::PhysObj::ApplyTorqueImpulse);
	classDef.def("ClearForces", &::PhysObj::ClearForces);
	classDef.def("GetTotalForce", &::PhysObj::GetTotalForce);
	classDef.def("GetTotalTorque", &::PhysObj::GetTotalTorque);
	classDef.def("GetPos", &::PhysObj::GetPosition);
	classDef.def("SetPos", &::PhysObj::SetPosition);
	classDef.def("GetRotation", &::PhysObj::GetOrientation);
	classDef.def("SetRotation", &::PhysObj::SetOrientation);
	classDef.def("GetBounds", &::PhysObj::GetAABB, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});

	classDef.def("SetLinearSleepingThreshold", &::PhysObj::SetLinearSleepingThreshold);
	classDef.def("SetAngularSleepingThreshold", &::PhysObj::SetAngularSleepingThreshold);
	classDef.def("SetSleepingThresholds", &::PhysObj::SetSleepingThresholds);
	classDef.def("GetLinearSleepingThreshold", &::PhysObj::GetLinearSleepingThreshold);
	classDef.def("GetAngularSleepingThreshold", &::PhysObj::GetAngularSleepingThreshold);
	classDef.def("GetSleepingThreshold", &::PhysObj::GetSleepingThreshold);

	classDef.def("IsOnGround", &IsOnGround);
	classDef.def("IsGroundWalkable", static_cast<bool (*)(lua_State *, ::PhysObj &)>([](lua_State *l, ::PhysObj &physObj) {
		if(physObj.IsController() == false)
			return false;
		return static_cast<ControllerPhysObj &>(physObj).IsGroundWalkable();
	}));
	classDef.def("GetGroundEntity", static_cast<BaseEntity *(*)(lua_State *, ::PhysObj &)>([](lua_State *l, ::PhysObj &physObj) -> BaseEntity * {
		if(physObj.IsController() == false)
			return nullptr;
		return static_cast<ControllerPhysObj &>(physObj).GetGroundEntity();
	}));
	classDef.def("GetGroundPhysObject", static_cast<::PhysObj *(*)(lua_State *, ::PhysObj &)>([](lua_State *l, ::PhysObj &physObj) -> ::PhysObj * {
		if(physObj.IsController() == false)
			return nullptr;
		return static_cast<ControllerPhysObj *>(&physObj)->GetGroundPhysObject();
	}));
	classDef.def("GetGroundPhysCollisionObject", static_cast<pragma::physics::ICollisionObject *(*)(lua_State *, ::PhysObj &)>([](lua_State *l, ::PhysObj &physObj) -> pragma::physics::ICollisionObject * {
		if(physObj.IsController() == false)
			return nullptr;
		return static_cast<ControllerPhysObj *>(&physObj)->GetGroundPhysCollisionObject();
	}));
	classDef.def("GetGroundSurfaceMaterial", static_cast<int32_t (*)(lua_State *, ::PhysObj &)>([](lua_State *l, ::PhysObj &physObj) {
		if(physObj.IsController() == false)
			return -1;
		return static_cast<ControllerPhysObj &>(physObj).GetGroundSurfaceMaterial();
	}));
	classDef.def("GetGroundVelocity", static_cast<Vector3 (*)(lua_State *, ::PhysObj &)>([](lua_State *l, ::PhysObj &physObj) -> Vector3 {
		if(physObj.IsController() == false)
			return {};
		return static_cast<ControllerPhysObj &>(physObj).GetGroundVelocity();
	}));
	classDef.def("GetGroundFriction", static_cast<float (*)(lua_State *, ::PhysObj &)>([](lua_State *l, ::PhysObj &physObj) {
		auto *physMat = physObj.IsController() ? static_cast<ControllerPhysObj &>(physObj).GetController()->GetGroundMaterial() : nullptr;
		if(physMat == nullptr)
			return 1.f;
		return physMat->GetDynamicFriction();
	}));
	mod[classDef];
}

bool Lua::PhysObj::IsValid(PhysObjHandle &hPhysObj) { return hPhysObj.IsValid(); }
luabind::tableT<pragma::physics::ICollisionObject> Lua::PhysObj::GetCollisionObjects(lua_State *l, ::PhysObj &physObj)
{
	auto &objs = physObj.GetCollisionObjects();
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &o : objs) {
		if(o.IsExpired())
			continue;
		t[idx++] = o->GetLuaObject();
	}
	return t;
}
void Lua::PhysObj::SetCollisionFilterGroup(lua_State *l, ::PhysObj &physObj, int group) { physObj.SetCollisionFilter(physObj.GetCollisionFilter(), static_cast<CollisionMask>(group)); }
bool Lua::PhysObj::IsOnGround(lua_State *l, ::PhysObj &physObj)
{

	if(physObj.IsController() == false)
		return false;
	return static_cast<ControllerPhysObj &>(physObj).IsOnGround();
}
