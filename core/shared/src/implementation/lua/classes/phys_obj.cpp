// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/ostream_operator_alias.hpp"

module pragma.shared;

import :scripting.lua.classes.phys_obj;

namespace Lua {
	namespace PhysObj {
		static bool IsValid(PhysObjHandle &hPhysObj);
		static luabind::tableT<pragma::physics::ICollisionObject> GetCollisionObjects(lua::State *l, pragma::physics::PhysObj &physObj);
		static void SetCollisionFilterGroup(lua::State *l, pragma::physics::PhysObj &physObj, int group);

		static bool IsOnGround(lua::State *l, pragma::physics::PhysObj &physObj);
	};
};

static std::ostream &operator<<(std::ostream &out, const pragma::physics::PhysObj *physObj)
{
	if(!physObj)
		out << "PhysObj[NULL]";
	else
		operator<<(out, *physObj);
	return out;
}

#ifdef __clang__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::physics, PhysObj);
#endif

void Lua::PhysObj::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<pragma::physics::PhysObj>("Object");
	classDef.def(luabind::tostring(luabind::self));
	classDef.def("IsValid", &IsValid);
	classDef.def("SetLinearVelocity", &pragma::physics::PhysObj::SetLinearVelocity);
	classDef.def("GetLinearVelocity", &pragma::physics::PhysObj::GetLinearVelocity);
	classDef.def("AddLinearVelocity", &pragma::physics::PhysObj::AddLinearVelocity);
	classDef.def("SetAngularVelocity", &pragma::physics::PhysObj::SetAngularVelocity);
	classDef.def("GetAngularVelocity", &pragma::physics::PhysObj::GetAngularVelocity);
	classDef.def("AddAngularVelocity", &pragma::physics::PhysObj::AddAngularVelocity);
	classDef.def("PutToSleep", &pragma::physics::PhysObj::PutToSleep);
	classDef.def("WakeUp", &pragma::physics::PhysObj::WakeUp);
	classDef.def("GetMass", &pragma::physics::PhysObj::GetMass);
	classDef.def("SetMass", &pragma::physics::PhysObj::SetMass);
	classDef.def("GetLinearFactor", &pragma::physics::PhysObj::GetLinearFactor);
	classDef.def("GetAngularFactor", &pragma::physics::PhysObj::GetAngularFactor);
	classDef.def("SetLinearFactor", &pragma::physics::PhysObj::SetLinearFactor);
	classDef.def("SetAngularFactor", &pragma::physics::PhysObj::SetAngularFactor);
	classDef.def("GetCollisionObjects", &GetCollisionObjects);
	classDef.def("GetOwner", &pragma::physics::PhysObj::GetOwner);
	classDef.def("SetCollisionFilterMask", &pragma::physics::PhysObj::SetCollisionFilterMask);
	classDef.def("GetCollisionFilterMask", &pragma::physics::PhysObj::GetCollisionFilterMask);
	classDef.def("SetCollisionFilterGroup", &SetCollisionFilterGroup);
	classDef.def("GetCollisionFilterGroup", static_cast<pragma::physics::CollisionMask (pragma::physics::PhysObj::*)() const>(&pragma::physics::PhysObj::GetCollisionFilter));
	classDef.def("SetCollisionFilter", static_cast<void (pragma::physics::PhysObj::*)(pragma::physics::CollisionMask, pragma::physics::CollisionMask)>(&pragma::physics::PhysObj::SetCollisionFilter));
	classDef.def("SetDamping", &pragma::physics::PhysObj::SetDamping);
	classDef.def("SetLinearDamping", &pragma::physics::PhysObj::SetLinearDamping);
	classDef.def("SetAngularDamping", &pragma::physics::PhysObj::SetAngularDamping);
	classDef.def("GetLinearDamping", &pragma::physics::PhysObj::GetLinearDamping);
	classDef.def("GetAngularDamping", &pragma::physics::PhysObj::GetAngularDamping);
	classDef.def("ApplyForce", static_cast<void (pragma::physics::PhysObj::*)(const Vector3 &)>(&pragma::physics::PhysObj::ApplyForce));
	classDef.def("ApplyForce", static_cast<void (pragma::physics::PhysObj::*)(const Vector3 &, const Vector3 &)>(&pragma::physics::PhysObj::ApplyForce));
	classDef.def("ApplyImpulse", static_cast<void (pragma::physics::PhysObj::*)(const Vector3 &)>(&pragma::physics::PhysObj::ApplyImpulse));
	classDef.def("ApplyImpulse", static_cast<void (pragma::physics::PhysObj::*)(const Vector3 &, const Vector3 &)>(&pragma::physics::PhysObj::ApplyImpulse));
	classDef.def("ApplyTorque", &pragma::physics::PhysObj::ApplyTorque);
	classDef.def("ApplyTorqueImpulse", &pragma::physics::PhysObj::ApplyTorqueImpulse);
	classDef.def("ClearForces", &pragma::physics::PhysObj::ClearForces);
	classDef.def("GetTotalForce", &pragma::physics::PhysObj::GetTotalForce);
	classDef.def("GetTotalTorque", &pragma::physics::PhysObj::GetTotalTorque);
	classDef.def("GetPos", &pragma::physics::PhysObj::GetPosition);
	classDef.def("SetPos", &pragma::physics::PhysObj::SetPosition);
	classDef.def("GetRotation", &pragma::physics::PhysObj::GetOrientation);
	classDef.def("SetRotation", &pragma::physics::PhysObj::SetOrientation);
	classDef.def("GetBounds", &pragma::physics::PhysObj::GetAABB, luabind::meta::join<luabind::out_value<2>, luabind::out_value<3>>::type {});

	classDef.def("SetLinearSleepingThreshold", &pragma::physics::PhysObj::SetLinearSleepingThreshold);
	classDef.def("SetAngularSleepingThreshold", &pragma::physics::PhysObj::SetAngularSleepingThreshold);
	classDef.def("SetSleepingThresholds", &pragma::physics::PhysObj::SetSleepingThresholds);
	classDef.def("GetLinearSleepingThreshold", &pragma::physics::PhysObj::GetLinearSleepingThreshold);
	classDef.def("GetAngularSleepingThreshold", &pragma::physics::PhysObj::GetAngularSleepingThreshold);
	classDef.def("GetSleepingThreshold", &pragma::physics::PhysObj::GetSleepingThreshold);

	classDef.def("IsOnGround", &IsOnGround);
	classDef.def("IsGroundWalkable", static_cast<bool (*)(lua::State *, pragma::physics::PhysObj &)>([](lua::State *l, pragma::physics::PhysObj &physObj) {
		if(physObj.IsController() == false)
			return false;
		return static_cast<pragma::physics::ControllerPhysObj &>(physObj).IsGroundWalkable();
	}));
	classDef.def("GetGroundEntity", static_cast<pragma::ecs::BaseEntity *(*)(lua::State *, pragma::physics::PhysObj &)>([](lua::State *l, pragma::physics::PhysObj &physObj) -> pragma::ecs::BaseEntity * {
		if(physObj.IsController() == false)
			return nullptr;
		return static_cast<pragma::physics::ControllerPhysObj &>(physObj).GetGroundEntity();
	}));
	classDef.def("GetGroundPhysObject", static_cast<pragma::physics::PhysObj *(*)(lua::State *, pragma::physics::PhysObj &)>([](lua::State *l, pragma::physics::PhysObj &physObj) -> pragma::physics::PhysObj * {
		if(physObj.IsController() == false)
			return nullptr;
		return static_cast<pragma::physics::ControllerPhysObj *>(&physObj)->GetGroundPhysObject();
	}));
	classDef.def("GetGroundPhysCollisionObject", static_cast<pragma::physics::ICollisionObject *(*)(lua::State *, pragma::physics::PhysObj &)>([](lua::State *l, pragma::physics::PhysObj &physObj) -> pragma::physics::ICollisionObject * {
		if(physObj.IsController() == false)
			return nullptr;
		return static_cast<pragma::physics::ControllerPhysObj *>(&physObj)->GetGroundPhysCollisionObject();
	}));
	classDef.def("GetGroundSurfaceMaterial", static_cast<int32_t (*)(lua::State *, pragma::physics::PhysObj &)>([](lua::State *l, pragma::physics::PhysObj &physObj) {
		if(physObj.IsController() == false)
			return -1;
		return static_cast<pragma::physics::ControllerPhysObj &>(physObj).GetGroundSurfaceMaterial();
	}));
	classDef.def("GetGroundVelocity", static_cast<Vector3 (*)(lua::State *, pragma::physics::PhysObj &)>([](lua::State *l, pragma::physics::PhysObj &physObj) -> Vector3 {
		if(physObj.IsController() == false)
			return {};
		return static_cast<pragma::physics::ControllerPhysObj &>(physObj).GetGroundVelocity();
	}));
	classDef.def("GetGroundFriction", static_cast<float (*)(lua::State *, pragma::physics::PhysObj &)>([](lua::State *l, pragma::physics::PhysObj &physObj) {
		auto *physMat = physObj.IsController() ? static_cast<pragma::physics::ControllerPhysObj &>(physObj).GetController()->GetGroundMaterial() : nullptr;
		if(physMat == nullptr)
			return 1.f;
		return physMat->GetDynamicFriction();
	}));
	mod[classDef];
}

bool Lua::PhysObj::IsValid(PhysObjHandle &hPhysObj) { return hPhysObj.IsValid(); }
luabind::tableT<pragma::physics::ICollisionObject> Lua::PhysObj::GetCollisionObjects(lua::State *l, pragma::physics::PhysObj &physObj)
{
	auto &objs = physObj.GetCollisionObjects();
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto &o : objs) {
		if(o.IsExpired())
			continue;
		t[idx++] = o->GetLuaObject(l);
	}
	return t;
}
void Lua::PhysObj::SetCollisionFilterGroup(lua::State *l, pragma::physics::PhysObj &physObj, int group) { physObj.SetCollisionFilter(physObj.GetCollisionFilter(), static_cast<pragma::physics::CollisionMask>(group)); }
bool Lua::PhysObj::IsOnGround(lua::State *l, pragma::physics::PhysObj &physObj)
{

	if(physObj.IsController() == false)
		return false;
	return static_cast<pragma::physics::ControllerPhysObj &>(physObj).IsOnGround();
}
