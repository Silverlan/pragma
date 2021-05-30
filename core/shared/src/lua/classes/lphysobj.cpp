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
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/physics/controller.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/base_physics_component.hpp"

class PhysObjHandle;
namespace Lua
{
	namespace PhysObj
	{
		static void IsValid(lua_State *l,PhysObjHandle &hPhysObj);
		static void SetLinearVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel);
		static void GetLinearVelocity(lua_State *l,PhysObjHandle &hPhysObj);
		static void AddLinearVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel);
		static void SetAngularVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel);
		static void GetAngularVelocity(lua_State *l,PhysObjHandle &hPhysObj);
		static void AddAngularVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel);
		static void PutToSleep(lua_State *l,PhysObjHandle &hPhysObj);
		static void WakeUp(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetActor(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetActors(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetMass(lua_State *l,PhysObjHandle &hPhysObj);
		static void SetMass(lua_State *l,PhysObjHandle &hPhysObj,float mass);
		static void SetLinearFactor(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &factor);
		static void SetAngularFactor(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &factor);
		static void GetLinearFactor(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetAngularFactor(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetCollisionObjects(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetOwner(lua_State *l,PhysObjHandle &hPhysObj);
		static void SetCollisionFilterMask(lua_State *l,PhysObjHandle &hPhysObj,int mask);
		static void GetCollisionFilterMask(lua_State *l,PhysObjHandle &hPhysObj);
		static void SetCollisionFilterGroup(lua_State *l,PhysObjHandle &hPhysObj,int group);
		static void GetCollisionFilterGroup(lua_State *l,PhysObjHandle &hPhysObj);
		static void SetCollisionFilter(lua_State *l,PhysObjHandle &hPhysObj,int mask,int group);

		static void SetDamping(lua_State *l,PhysObjHandle &hPhysObj,float linDamping,float angDamping);
		static void SetLinearDamping(lua_State *l,PhysObjHandle &hPhysObj,float linDamping);
		static void SetAngularDamping(lua_State *l,PhysObjHandle &hPhysObj,float angDamping);
		static void GetLinearDamping(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetAngularDamping(lua_State *l,PhysObjHandle &hPhysObj);

		static void ApplyForce(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &force);
		static void ApplyForce(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &force,Vector3 &relPos);
		static void ApplyImpulse(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &impulse);
		static void ApplyImpulse(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &impulse,Vector3 &relPos);
		static void ApplyTorque(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &torque);
		static void ApplyTorqueImpulse(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &torque);
		static void ClearForces(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetTotalForce(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetTotalTorque(lua_State *l,PhysObjHandle &hPhysObj);

		static void GetPos(lua_State *l,PhysObjHandle &hPhysObj);
		static void SetPos(lua_State *l,PhysObjHandle &hPhysObj,const Vector3 &pos);
		static void GetRotation(lua_State *l,PhysObjHandle &hPhysObj);
		static void SetRotation(lua_State *l,PhysObjHandle &hPhysObj,const Quat &rot);
		static void GetBounds(lua_State *l,PhysObjHandle &hPhysObj);

		static void SetLinearSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj,float threshold);
		static void SetAngularSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj,float threshold);
		static void SetSleepingThresholds(lua_State *l,PhysObjHandle &hPhysObj,float linear,float angular);
		static void GetLinearSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetAngularSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj);
		static void GetSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj);

		static void IsOnGround(lua_State *l,PhysObjHandle &hPhysObj);
	};
};

static std::ostream &operator<<(std::ostream &out,const PhysObjHandle &o)
{
	if(!o.IsValid())
		out<<"PhysObj[NULL]";
	else
		operator<<(out,*o.get());
	return out;
}

void Lua::PhysObj::register_class(lua_State *l,luabind::module_ &mod)
{
	auto classDef = luabind::class_<PhysObjHandle>("Object");
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid",&IsValid);
	classDef.def("SetLinearVelocity",&SetLinearVelocity);
	classDef.def("GetLinearVelocity",&GetLinearVelocity);
	classDef.def("AddLinearVelocity",&AddLinearVelocity);
	classDef.def("SetAngularVelocity",&SetAngularVelocity);
	classDef.def("GetAngularVelocity",&GetAngularVelocity);
	classDef.def("AddAngularVelocity",&AddAngularVelocity);
	classDef.def("PutToSleep",&PutToSleep);
	classDef.def("WakeUp",&WakeUp);
	classDef.def("GetMass",&GetMass);
	classDef.def("SetMass",&SetMass);
	classDef.def("GetLinearFactor",&GetLinearFactor);
	classDef.def("GetAngularFactor",&GetAngularFactor);
	classDef.def("SetLinearFactor",&SetLinearFactor);
	classDef.def("SetAngularFactor",&SetAngularFactor);
	classDef.def("GetCollisionObjects",&GetCollisionObjects);
	classDef.def("GetOwner",&GetOwner);
	classDef.def("SetCollisionFilterMask",&SetCollisionFilterMask);
	classDef.def("GetCollisionFilterMask",&GetCollisionFilterMask);
	classDef.def("SetCollisionFilterGroup",&SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup",&GetCollisionFilterGroup);
	classDef.def("SetCollisionFilter",&SetCollisionFilter);
	classDef.def("SetDamping",&SetDamping);
	classDef.def("SetLinearDamping",&SetLinearDamping);
	classDef.def("SetAngularDamping",&SetAngularDamping);
	classDef.def("GetLinearDamping",&GetLinearDamping);
	classDef.def("GetAngularDamping",&GetAngularDamping);
	classDef.def("ApplyForce",static_cast<void(*)(lua_State*,PhysObjHandle&,Vector3&)>(&ApplyForce));
	classDef.def("ApplyForce",static_cast<void(*)(lua_State*,PhysObjHandle&,Vector3&,Vector3&)>(&ApplyForce));
	classDef.def("ApplyImpulse",static_cast<void(*)(lua_State*,PhysObjHandle&,Vector3&)>(&ApplyImpulse));
	classDef.def("ApplyImpulse",static_cast<void(*)(lua_State*,PhysObjHandle&,Vector3&,Vector3&)>(&ApplyImpulse));
	classDef.def("ApplyTorque",&ApplyTorque);
	classDef.def("ApplyTorqueImpulse",&ApplyTorqueImpulse);
	classDef.def("ClearForces",&ClearForces);
	classDef.def("GetTotalForce",&GetTotalForce);
	classDef.def("GetTotalTorque",&GetTotalTorque);
	classDef.def("GetPos",&GetPos);
	classDef.def("SetPos",&SetPos);
	classDef.def("GetRotation",&GetRotation);
	classDef.def("SetRotation",&SetRotation);
	classDef.def("GetBounds",&GetBounds);

	classDef.def("SetLinearSleepingThreshold",&SetLinearSleepingThreshold);
	classDef.def("SetAngularSleepingThreshold",&SetAngularSleepingThreshold);
	classDef.def("SetSleepingThresholds",&SetSleepingThresholds);
	classDef.def("GetLinearSleepingThreshold",&GetLinearSleepingThreshold);
	classDef.def("GetAngularSleepingThreshold",&GetAngularSleepingThreshold);
	classDef.def("GetSleepingThreshold",&GetSleepingThreshold);

	classDef.def("IsOnGround",&IsOnGround);
	classDef.def("IsGroundWalkable",static_cast<void(*)(lua_State*,PhysObjHandle&)>([](lua_State *l,PhysObjHandle &hPhysObj) {
		LUA_CHECK_PHYSOBJ(l,hPhysObj);
		if(hPhysObj->IsController() == false)
		{
			Lua::PushBool(l,false);
			return;
		}
		Lua::PushBool(l,static_cast<ControllerPhysObj*>(hPhysObj.get())->IsGroundWalkable());
	}));
	classDef.def("GetGroundEntity",static_cast<void(*)(lua_State*,PhysObjHandle&)>([](lua_State *l,PhysObjHandle &hPhysObj) {
		LUA_CHECK_PHYSOBJ(l,hPhysObj);
		if(hPhysObj->IsController() == false)
			return;
		auto *ent = static_cast<ControllerPhysObj*>(hPhysObj.get())->GetGroundEntity();
		if(ent == nullptr)
			return;
		ent->GetLuaObject()->push(l);
	}));
	classDef.def("GetGroundPhysObject",static_cast<void(*)(lua_State*,PhysObjHandle&)>([](lua_State *l,PhysObjHandle &hPhysObj) {
		LUA_CHECK_PHYSOBJ(l,hPhysObj);
		if(hPhysObj->IsController() == false)
			return;
		auto *physObj = static_cast<ControllerPhysObj*>(hPhysObj.get())->GetGroundPhysObject();
		if(physObj == nullptr)
			return;
		luabind::object(l,physObj->GetHandle()).push(l);
	}));
	classDef.def("GetGroundPhysCollisionObject",static_cast<void(*)(lua_State*,PhysObjHandle&)>([](lua_State *l,PhysObjHandle &hPhysObj) {
		LUA_CHECK_PHYSOBJ(l,hPhysObj);
		if(hPhysObj->IsController() == false)
			return;
		auto *colObj = static_cast<ControllerPhysObj*>(hPhysObj.get())->GetGroundPhysCollisionObject();
		if(colObj == nullptr)
			return;
		luabind::object(l,colObj->GetHandle()).push(l);
	}));
	classDef.def("GetGroundSurfaceMaterial",static_cast<void(*)(lua_State*,PhysObjHandle&)>([](lua_State *l,PhysObjHandle &hPhysObj) {
		LUA_CHECK_PHYSOBJ(l,hPhysObj);
		if(hPhysObj->IsController() == false)
		{
			Lua::PushInt(l,-1);
			return;
		}
		Lua::PushInt(l,static_cast<ControllerPhysObj*>(hPhysObj.get())->GetGroundSurfaceMaterial());
	}));
	classDef.def("GetGroundVelocity",static_cast<void(*)(lua_State*,PhysObjHandle&)>([](lua_State *l,PhysObjHandle &hPhysObj) {
		LUA_CHECK_PHYSOBJ(l,hPhysObj);
		if(hPhysObj->IsController() == false)
		{
			Lua::Push<Vector3>(l,Vector3{});
			return;
		}
		Lua::Push<Vector3>(l,static_cast<ControllerPhysObj*>(hPhysObj.get())->GetGroundVelocity());
	}));
	classDef.def("GetGroundFriction",static_cast<void(*)(lua_State*,PhysObjHandle&)>([](lua_State *l,PhysObjHandle &hPhysObj) {
		LUA_CHECK_PHYSOBJ(l,hPhysObj);
		auto *physMat = hPhysObj->IsController() ? static_cast<ControllerPhysObj*>(hPhysObj.get())->GetController()->GetGroundMaterial() : nullptr;
		if(physMat == nullptr)
		{
			Lua::PushNumber(l,1.0);
			return;
		}
		Lua::PushNumber(l,physMat->GetDynamicFriction());
	}));
	mod[classDef];
}

void Lua::PhysObj::IsValid(lua_State *l,PhysObjHandle &hPhysObj)
{
	lua_pushboolean(l,hPhysObj.IsValid() ? true : false);
}
void Lua::PhysObj::SetLinearVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetLinearVelocity(vel);
}
void Lua::PhysObj::GetLinearVelocity(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Vector3 vel = hPhysObj->GetLinearVelocity();
	Lua::Push<Vector3>(l,vel);
}
void Lua::PhysObj::AddLinearVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->AddLinearVelocity(vel);
}
void Lua::PhysObj::SetAngularVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetAngularVelocity(vel);
}
void Lua::PhysObj::GetAngularVelocity(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Vector3 vel = hPhysObj->GetAngularVelocity();
	Lua::Push<Vector3>(l,vel);
}
void Lua::PhysObj::AddAngularVelocity(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &vel)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->AddAngularVelocity(vel);
}
void Lua::PhysObj::PutToSleep(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->PutToSleep();
}
void Lua::PhysObj::WakeUp(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->WakeUp();
}
void Lua::PhysObj::GetMass(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::PushNumber(l,hPhysObj->GetMass());
}
void Lua::PhysObj::SetMass(lua_State *l,PhysObjHandle &hPhysObj,float mass)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetMass(mass);
}
void Lua::PhysObj::SetLinearFactor(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &factor)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetLinearFactor(factor);
}
void Lua::PhysObj::SetAngularFactor(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &factor)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetAngularFactor(factor);
}
void Lua::PhysObj::GetLinearFactor(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::Push<Vector3>(l,hPhysObj->GetLinearFactor());
}
void Lua::PhysObj::GetAngularFactor(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::Push<Vector3>(l,hPhysObj->GetAngularFactor());
}
void Lua::PhysObj::GetCollisionObjects(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	auto &objs = hPhysObj->GetCollisionObjects();
	lua_newtable(l);
	int top = lua_gettop(l);
	int n = 1;
	for(auto it=objs.begin();it!=objs.end();++it)
	{
		auto &o = *it;
		if(o.IsValid())
		{
			o->Push(l);
			lua_rawseti(l,top,n);
			n++;
		}
	}
}
void Lua::PhysObj::GetOwner(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	auto *owner = hPhysObj->GetOwner();
	if(owner == nullptr)
		return;
	owner->PushLuaObject(l);
}
void Lua::PhysObj::SetCollisionFilterMask(lua_State *l,PhysObjHandle &hPhysObj,int mask)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetCollisionFilterMask(static_cast<CollisionMask>(mask));
}
void Lua::PhysObj::GetCollisionFilterMask(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::PushInt(l,hPhysObj->GetCollisionFilterMask());
}
void Lua::PhysObj::SetCollisionFilterGroup(lua_State *l,PhysObjHandle &hPhysObj,int group)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetCollisionFilter(hPhysObj->GetCollisionFilter(),static_cast<CollisionMask>(group));
}
void Lua::PhysObj::GetCollisionFilterGroup(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::PushInt(l,hPhysObj->GetCollisionFilter());
}
void Lua::PhysObj::SetCollisionFilter(lua_State *l,PhysObjHandle &hPhysObj,int mask,int group)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetCollisionFilter(static_cast<CollisionMask>(mask),static_cast<CollisionMask>(group));
}
void Lua::PhysObj::SetDamping(lua_State *l,PhysObjHandle &hPhysObj,float linDamping,float angDamping)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetDamping(linDamping,angDamping);
}
void Lua::PhysObj::SetLinearDamping(lua_State *l,PhysObjHandle &hPhysObj,float linDamping)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetLinearDamping(linDamping);
}
void Lua::PhysObj::SetAngularDamping(lua_State *l,PhysObjHandle &hPhysObj,float angDamping)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetAngularDamping(angDamping);
}
void Lua::PhysObj::GetLinearDamping(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::PushNumber(l,hPhysObj->GetLinearDamping());
}
void Lua::PhysObj::GetAngularDamping(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::PushNumber(l,hPhysObj->GetAngularDamping());
}

void Lua::PhysObj::ApplyForce(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &force)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->ApplyForce(force);
}
void Lua::PhysObj::ApplyForce(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &force,Vector3 &relPos)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->ApplyForce(force,relPos);
}
void Lua::PhysObj::ApplyImpulse(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &impulse)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->ApplyImpulse(impulse);
}
void Lua::PhysObj::ApplyImpulse(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &torque,Vector3 &relPos)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->ApplyImpulse(torque,relPos);
}
void Lua::PhysObj::ApplyTorqueImpulse(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &torque)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->ApplyTorqueImpulse(torque);
}
void Lua::PhysObj::ApplyTorque(lua_State *l,PhysObjHandle &hPhysObj,Vector3 &torque)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->ApplyTorque(torque);
}
void Lua::PhysObj::ClearForces(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->ClearForces();
}
void Lua::PhysObj::GetTotalForce(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::Push<Vector3>(l,hPhysObj->GetTotalForce());
}
void Lua::PhysObj::GetTotalTorque(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::Push<Vector3>(l,hPhysObj->GetTotalTorque());
}

void Lua::PhysObj::GetPos(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::Push<Vector3>(l,hPhysObj->GetPosition());
}
void Lua::PhysObj::SetPos(lua_State *l,PhysObjHandle &hPhysObj,const Vector3 &pos)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetPosition(pos);
}
void Lua::PhysObj::GetRotation(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::Push<Quat>(l,hPhysObj->GetOrientation());
}
void Lua::PhysObj::SetRotation(lua_State *l,PhysObjHandle &hPhysObj,const Quat &rot)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetOrientation(rot);
}
void Lua::PhysObj::GetBounds(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Vector3 min,max;
	hPhysObj->GetAABB(min,max);
	Lua::Push<Vector3>(l,min);
	Lua::Push<Vector3>(l,max);
}
void Lua::PhysObj::SetLinearSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj,float threshold)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetLinearSleepingThreshold(threshold);
}
void Lua::PhysObj::SetAngularSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj,float threshold)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetAngularSleepingThreshold(threshold);
}
void Lua::PhysObj::SetSleepingThresholds(lua_State *l,PhysObjHandle &hPhysObj,float linear,float angular)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	hPhysObj->SetSleepingThresholds(linear,angular);
}
void Lua::PhysObj::GetLinearSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::PushNumber(l,hPhysObj->GetLinearSleepingThreshold());
}
void Lua::PhysObj::GetAngularSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	Lua::PushNumber(l,hPhysObj->GetAngularSleepingThreshold());
}
void Lua::PhysObj::GetSleepingThreshold(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	auto thresholds = hPhysObj->GetSleepingThreshold();
	Lua::PushNumber(l,thresholds.first);
	Lua::PushNumber(l,thresholds.second);
}
void Lua::PhysObj::IsOnGround(lua_State *l,PhysObjHandle &hPhysObj)
{
	LUA_CHECK_PHYSOBJ(l,hPhysObj);
	if(hPhysObj->IsController() == false)
	{
		Lua::PushBool(l,false);
		return;
	}
	Lua::PushBool(l,static_cast<ControllerPhysObj*>(hPhysObj.get())->IsOnGround());
}
