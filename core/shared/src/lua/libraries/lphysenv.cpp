/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "luasystem.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/lua/classes/lphysobj.h"
#include "pragma/physics/environment.hpp"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_quaternion.h"
#include "pragma/lua/classes/ldef_tracedata.h"
#include "pragma/lua/classes/ldef_mat4.h"
#include "pragma/lua/libraries/lray.h"
#include "pragma/physics/raytraces.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/classes/lphyssoftbodyinfo.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/physics/phys_material.hpp"
#include "pragma/physics/vehicle.hpp"
#include "pragma/physics/controller.hpp"
#include "pragma/physics/ik/util_ik.hpp"
#include "pragma/buss_ik/Tree.h"
#include "pragma/buss_ik/Jacobian.h"
#include "pragma/physics/ik/ik_controller.hpp"
#include <mathutil/color.h>
#include <luainterface.hpp>
#include <luabind/iterator_policy.hpp>
#include <glm/gtx/matrix_decompose.hpp>

extern DLLENGINE Engine *engine;

// #define ENABLE_DEPRECATED_PHYSICS

namespace Lua
{
	namespace physenv
	{
		static int raycast(lua_State *l);
		static int sweep(lua_State *l);
		static int overlap(lua_State *l);
		static int create_rigid_body(lua_State *l);
		static int create_convex_hull_shape(lua_State *l);
		static int create_box_shape(lua_State *l);
		static int create_capsule_shape(lua_State *l);
		static int create_sphere_shape(lua_State *l);
		static int create_cylinder_shape(lua_State *l);
		static int create_compound_shape(lua_State *l);
		static int create_heightfield_terrain_shape(lua_State *l);
		static int create_character_controller(lua_State *l);
		static int create_fixed_constraint(lua_State *l);
		static int create_ball_socket_constraint(lua_State *l);
		static int create_hinge_constraint(lua_State *l);
		static int create_slider_constraint(lua_State *l);
		static int create_cone_twist_constraint(lua_State *l);
		static int create_DoF_constraint(lua_State *l);
		static int create_dof_spring_constraint(lua_State *l);
		static int create_surface_material(lua_State *l);
		static int create_box_controller(lua_State *l);
		static int create_capsule_controller(lua_State *l);
		static int create_material(lua_State *l);
		static int create_ghost_object(lua_State *l);
		static int create_plane(lua_State *l);
		static int get_surface_material(lua_State *l);
		static int get_surface_materials(lua_State *l);

		static int calc_torque_from_angular_velocity(lua_State *l);
		static int calc_angular_velocity_from_torque(lua_State *l);
		static int calc_force_from_linear_velocity(lua_State *l);
		static int calc_linear_velocity_from_force(lua_State *l);
	};
};

static void create_standard_four_wheel_drive(lua_State *l,luabind::object oWheelCenterOffsets,float handBrakeTorque=6'400'000.0,float maxSteeringAngle=60.0)
{
	auto tWheelCenterOffsets = 1;
	Lua::CheckTable(l,tWheelCenterOffsets);
	std::array<Vector3,pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> centerOffsets {};
	auto i = 1;
	for(auto &centerOffset : centerOffsets)
	{
		centerOffset = {};
		Lua::PushInt(l,i++);
		Lua::GetTableValue(l,tWheelCenterOffsets);
		if(Lua::IsSet(l,-1))
			centerOffset = *Lua::CheckVector(l,-1);
		Lua::Pop(l,1);
	}
	Lua::Push<pragma::physics::VehicleCreateInfo>(l,pragma::physics::VehicleCreateInfo::CreateStandardFourWheelDrive(centerOffsets,handBrakeTorque,maxSteeringAngle));
}

static std::ostream &operator<<(std::ostream &out,const umath::Transform &t)
{
	auto &origin = t.GetOrigin();
	auto &rot = t.GetRotation();
	auto ang = EulerAngles{rot};
	out<<"Transform["<<origin.x<<","<<origin.y<<","<<origin.z<<"]["<<ang.p<<","<<ang.y<<","<<ang.r<<"]";
	return out;
}
static std::ostream &operator<<(std::ostream &out,const umath::ScaledTransform &t)
{
	auto &origin = t.GetOrigin();
	auto &rot = t.GetRotation();
	auto ang = EulerAngles{rot};
	auto &scale = t.GetScale();
	out<<"ScaledTransform["<<origin.x<<","<<origin.y<<","<<origin.z<<"]["<<ang.p<<","<<ang.y<<","<<ang.r<<"]["<<scale.x<<","<<scale.y<<","<<scale.z<<"]";
	return out;
}
void Lua::physenv::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();
	const auto *libName = "phys";
	Lua::RegisterLibrary(l,libName,{
		//{"create_character_controller",create_character_controller},
		{"create_convex_hull_shape",create_convex_hull_shape},
		{"create_box_shape",create_box_shape},
		{"create_capsule_shape",create_capsule_shape},
		{"create_sphere_shape",create_sphere_shape},
		{"create_cylinder_shape",create_cylinder_shape},
		{"create_compound_shape",create_compound_shape},
		{"create_heightfield_terrain_shape",create_heightfield_terrain_shape},
		{"create_rigid_body",create_rigid_body},
		{"create_ghost_object",create_ghost_object},
		{"create_plane",create_plane},
		{"create_fixed_constraint",create_fixed_constraint},
		{"create_ballsocket_constraint",create_ball_socket_constraint},
		{"create_hinge_constraint",create_hinge_constraint},
		{"create_slider_constraint",create_slider_constraint},
		{"create_conetwist_constraint",create_cone_twist_constraint},
		{"create_dof_constraint",create_DoF_constraint},
		{"create_dof_spring_constraint",create_dof_spring_constraint},
		{"create_surface_material",create_surface_material},
		{"create_material",create_material},
		{"create_box_controller",create_box_controller},
		{"create_capsule_controller",create_capsule_controller},
		{"create_vehicle",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *state = engine->GetNetworkState(l);
			auto *game = state->GetGameState();
			auto *env = game->GetPhysicsEnvironment();
			if(env == nullptr)
				return 0;
			auto &vhcCreateInfo = Lua::Check<pragma::physics::VehicleCreateInfo>(l,1);
			auto vhc = env->CreateVehicle(vhcCreateInfo);
			if(vhc == nullptr)
				return 0;
			vhc->Push(l);
			return 1;
		})},
		{"get_surface_material",get_surface_material},
		{"get_surface_materials",get_surface_materials},
		{"get_generic_material",static_cast<int32_t(*)(lua_State*)>([](lua_State *l) -> int32_t {
			auto *state = engine->GetNetworkState(l);
			auto *game = state->GetGameState();
			auto *env = game->GetPhysicsEnvironment();
			if(env == nullptr)
				return 0;
			auto &mat = env->GetGenericMaterial();
			mat.Push(l);
			return 1;
		})},
		{"raycast",raycast},
		{"sweep",sweep},
		{"overlap",overlap},

		{"calc_torque_from_angular_velocity",calc_torque_from_angular_velocity},
		{"calc_angular_velocity_from_torque",calc_angular_velocity_from_torque},
		{"calc_force_from_linear_velocity",calc_force_from_linear_velocity},
		{"calc_linear_velocity_from_force",calc_linear_velocity_from_force},
	});

	Lua::RegisterLibraryEnums(l,libName,{
		{"ACTIVATION_STATE_ACTIVE_TAG",ACTIVE_TAG},
		{"ACTIVATION_STATE_DISABLE_DEACTIVATION",DISABLE_DEACTIVATION},
		{"ACTIVATION_STATE_DISABLE_SIMULATION",DISABLE_SIMULATION},
		{"ACTIVATION_STATE_ISLAND_SLEEPING",ISLAND_SLEEPING},
		{"ACTIVATION_STATE_WANTS_DEACTIVATION",WANTS_DEACTIVATION}
	});

	Lua::RegisterLibraryEnums(l,libName,{
		{"TYPE_NONE",umath::to_integral(PHYSICSTYPE::NONE)},
		{"TYPE_DYNAMIC",umath::to_integral(PHYSICSTYPE::DYNAMIC)},
		{"TYPE_STATIC",umath::to_integral(PHYSICSTYPE::STATIC)},
		{"TYPE_BOXCONTROLLER",umath::to_integral(PHYSICSTYPE::BOXCONTROLLER)},
		{"TYPE_CAPSULECONTROLLER",umath::to_integral(PHYSICSTYPE::CAPSULECONTROLLER)},
		{"TYPE_SOFTBODY",umath::to_integral(PHYSICSTYPE::SOFTBODY)},

		{"COLLISIONMASK_NONE",umath::to_integral(CollisionMask::None)},
		{"COLLISIONMASK_STATIC",umath::to_integral(CollisionMask::Static)},
		{"COLLISIONMASK_DYNAMIC",umath::to_integral(CollisionMask::Dynamic)},
		{"COLLISIONMASK_GENERIC",umath::to_integral(CollisionMask::Generic)},
		{"COLLISIONMASK_PLAYER",umath::to_integral(CollisionMask::Player)},
		{"COLLISIONMASK_NPC",umath::to_integral(CollisionMask::NPC)},
		{"COLLISIONMASK_VEHICLE",umath::to_integral(CollisionMask::Vehicle)},
		{"COLLISIONMASK_ITEM",umath::to_integral(CollisionMask::Item)},
		{"COLLISIONMASK_ALL",umath::to_integral(CollisionMask::All)},
		{"COLLISIONMASK_DEFAULT",umath::to_integral(CollisionMask::Default)},
		{"COLLISIONMASK_PARTICLE",umath::to_integral(CollisionMask::Particle)},
		{"COLLISIONMASK_CHARACTER",umath::to_integral(CollisionMask::Character)},
		{"COLLISIONMASK_TRIGGER",umath::to_integral(CollisionMask::Trigger)},
		{"COLLISIONMASK_WATER",umath::to_integral(CollisionMask::Water)},
		{"COLLISIONMASK_WATER_SURFACE",umath::to_integral(CollisionMask::WaterSurface)},
		{"COLLISIONMASK_PLAYER_HITBOX",umath::to_integral(CollisionMask::PlayerHitbox)},
		{"COLLISIONMASK_NPC_HITBOX",umath::to_integral(CollisionMask::NPCHitbox)},
		{"COLLISIONMASK_CHARACTER_HITBOX",umath::to_integral(CollisionMask::CharacterHitbox)},
		{"COLLISIONMASK_ALL_HITBOX",umath::to_integral(CollisionMask::AllHitbox)},
		{"COLLISIONMASK_NO_COLLISION",umath::to_integral(CollisionMask::NoCollision)},

		{"RAYCAST_FLAG_BIT_REPORT_HIT_POSITION",umath::to_integral(RayCastFlags::ReportHitPosition)},
		{"RAYCAST_FLAG_BIT_REPORT_HIT_NORMAL",umath::to_integral(RayCastFlags::ReportHitNormal)},
		{"RAYCAST_FLAG_BIT_REPORT_HIT_UV",umath::to_integral(RayCastFlags::ReportHitUV)},
		{"RAYCAST_FLAG_BIT_REPORT_ALL_RESULTS",umath::to_integral(RayCastFlags::ReportAllResults)},
		{"RAYCAST_FLAG_BIT_REPORT_ANY_RESULT",umath::to_integral(RayCastFlags::ReportAnyResult)},
		{"RAYCAST_FLAG_BIT_REPORT_BACK_FACE_HITS",umath::to_integral(RayCastFlags::ReportBackFaceHits)},
		{"RAYCAST_FLAG_BIT_PRECISE",umath::to_integral(RayCastFlags::Precise)},
		{"RAYCAST_FLAG_BIT_IGNORE_DYNAMIC",umath::to_integral(RayCastFlags::IgnoreDynamic)},
		{"RAYCAST_FLAG_BIT_IGNORE_STATIC",umath::to_integral(RayCastFlags::IgnoreStatic)},
		{"RAYCAST_FLAG_BIT_INVERT_FILTER",umath::to_integral(RayCastFlags::InvertFilter)},
		{"RAYCAST_FLAG_DEFAULT",umath::to_integral(RayCastFlags::Default)},
		{"RAYCAST_FLAG_NONE",umath::to_integral(RayCastFlags::None)}
	});

	auto &physMod = lua.RegisterLibrary(libName);
	auto classBase = luabind::class_<pragma::physics::IBase>("Base");
	physMod[classBase];

	auto classDefCon = luabind::class_<pragma::physics::IController,pragma::physics::IBase>("Controller");
	classDefCon.def("IsValid",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		Lua::PushBool(l,hPhys != nullptr);
	}));
	classDefCon.def("Remove",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->ClaimOwnership().Remove();
	}));
	classDefCon.def("Spawn",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->Spawn();
	}));
	classDefCon.def("Move",static_cast<void(*)(lua_State*,pragma::physics::IController*,const Vector3&)>([](lua_State *l,pragma::physics::IController *hPhys,const Vector3 &disp) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto v = disp;
		hPhys->Move(v);
	}));
	classDefCon.def("GetCollisionFlags",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::PushInt(l,umath::to_integral(hPhys->GetCollisionFlags()));
	}));
	classDefCon.def("GetGroundShape",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto *groundShape = hPhys->GetGroundShape();
		if(groundShape == nullptr)
			return;
		groundShape->Push(l);
	}));
	classDefCon.def("GetGroundBody",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto *groundBody = hPhys->GetGroundBody();
		if(groundBody == nullptr)
			return;
		groundBody->Push(l);
	}));
	classDefCon.def("GetGroundMaterial",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto *groundMat = hPhys->GetGroundMaterial();
		if(groundMat == nullptr)
			return;
		groundMat->Push(l);
	}));
	classDefCon.def("IsTouchingGround",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::PushBool(l,hPhys->IsTouchingGround());
	}));
	classDefCon.def("GetGroundTouchPos",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto touchPos = hPhys->GetGroundTouchPos();
		if(touchPos.has_value() == false)
			return;
		Lua::Push<Vector3>(l,*touchPos);
	}));
	classDefCon.def("GetGroundTouchNormal",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto touchNormal = hPhys->GetGroundTouchNormal();
		if(touchNormal.has_value() == false)
			return;
		Lua::Push<Vector3>(l,*touchNormal);
	}));
	classDefCon.def("GetDimensions",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::Push<Vector3>(l,hPhys->GetDimensions());
	}));
	classDefCon.def("GetPos",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::Push<Vector3>(l,hPhys->GetPos());
	}));
	classDefCon.def("GetFootPos",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::Push<Vector3>(l,hPhys->GetFootPos());
	}));
	classDefCon.def("GetUpDirection",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::Push<Vector3>(l,hPhys->GetUpDirection());
	}));
	classDefCon.def("GetMoveVelocity",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::Push<Vector3>(l,hPhys->GetMoveVelocity());
	}));
	classDefCon.def("GetSlopeLimit",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::PushNumber(l,hPhys->GetSlopeLimit());
	}));
	classDefCon.def("GetStepHeight",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		Lua::PushNumber(l,hPhys->GetStepHeight());
	}));
	classDefCon.def("GetCollisionObject",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto *colObj = hPhys->GetCollisionObject();
		if(colObj == nullptr)
			return;
		colObj->Push(l);
	}));
	classDefCon.def("GetShape",static_cast<void(*)(lua_State*,pragma::physics::IController*)>([](lua_State *l,pragma::physics::IController *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		auto *shape = hPhys->GetShape();
		if(shape == nullptr)
			return;
		shape->Push(l);
	}));
	classDefCon.def("SetDimensions",static_cast<void(*)(lua_State*,pragma::physics::IController*,const Vector3&)>([](lua_State *l,pragma::physics::IController *hPhys,const Vector3 &dims) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->SetDimensions(dims);
	}));
	classDefCon.def("SetPos",static_cast<void(*)(lua_State*,pragma::physics::IController*,const Vector3&)>([](lua_State *l,pragma::physics::IController *hPhys,const Vector3 &pos) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->SetPos(pos);
	}));
	classDefCon.def("SetFootPos",static_cast<void(*)(lua_State*,pragma::physics::IController*,const Vector3&)>([](lua_State *l,pragma::physics::IController *hPhys,const Vector3 &pos) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->SetFootPos(pos);
	}));
	classDefCon.def("SetUpDirection",static_cast<void(*)(lua_State*,pragma::physics::IController*,const Vector3&)>([](lua_State *l,pragma::physics::IController *hPhys,const Vector3 &up) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->SetUpDirection(up);
	}));
	classDefCon.def("SetMoveVelocity",static_cast<void(*)(lua_State*,pragma::physics::IController*,const Vector3&)>([](lua_State *l,pragma::physics::IController *hPhys,const Vector3 &vel) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->SetMoveVelocity(vel);
	}));
	classDefCon.def("AddMoveVelocity",static_cast<void(*)(lua_State*,pragma::physics::IController*,const Vector3&)>([](lua_State *l,pragma::physics::IController *hPhys,const Vector3 &vel) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->AddMoveVelocity(vel);
	}));
	classDefCon.def("Resize",static_cast<void(*)(lua_State*,pragma::physics::IController*,float)>([](lua_State *l,pragma::physics::IController *hPhys,float newHeight) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->Resize(newHeight);
	}));
	classDefCon.def("SetSlopeLimit",static_cast<void(*)(lua_State*,pragma::physics::IController*,float)>([](lua_State *l,pragma::physics::IController *hPhys,float slopeLimit) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->SetSlopeLimit(slopeLimit);
	}));
	classDefCon.def("SetStepHeight",static_cast<void(*)(lua_State*,pragma::physics::IController*,float)>([](lua_State *l,pragma::physics::IController *hPhys,float stepHeight) {
		if(Lua::CheckHandle<pragma::physics::IController>(l,hPhys) == false)
			return;
		hPhys->SetStepHeight(stepHeight);
	}));
	physMod[classDefCon];

	auto classDefVhc = luabind::class_<pragma::physics::IVehicle,pragma::physics::IBase>("Vehicle");
	classDefVhc.add_static_constant("GEAR_REVERSE",umath::to_integral(pragma::physics::IVehicle::Gear::Reverse));
	classDefVhc.add_static_constant("GEAR_NEUTRAL",umath::to_integral(pragma::physics::IVehicle::Gear::Neutral));
	classDefVhc.add_static_constant("GEAR_FIRST",umath::to_integral(pragma::physics::IVehicle::Gear::First));
	classDefVhc.add_static_constant("GEAR_SECOND",umath::to_integral(pragma::physics::IVehicle::Gear::Second));
	classDefVhc.add_static_constant("GEAR_THIRD",umath::to_integral(pragma::physics::IVehicle::Gear::Third));
	classDefVhc.add_static_constant("GEAR_FOURTH",umath::to_integral(pragma::physics::IVehicle::Gear::Fourth));
	classDefVhc.add_static_constant("GEAR_FIFTH",umath::to_integral(pragma::physics::IVehicle::Gear::Fifth));
	classDefVhc.add_static_constant("GEAR_SIXTH",umath::to_integral(pragma::physics::IVehicle::Gear::Sixth));
	classDefVhc.add_static_constant("GEAR_SEVENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Seventh));
	classDefVhc.add_static_constant("GEAR_EIGHTH",umath::to_integral(pragma::physics::IVehicle::Gear::Eighth));
	classDefVhc.add_static_constant("GEAR_NINTH",umath::to_integral(pragma::physics::IVehicle::Gear::Ninth));
	classDefVhc.add_static_constant("GEAR_TENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Tenth));
	classDefVhc.add_static_constant("GEAR_ELEVENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Eleventh));
	classDefVhc.add_static_constant("GEAR_TWELFTH",umath::to_integral(pragma::physics::IVehicle::Gear::Twelfth));
	classDefVhc.add_static_constant("GEAR_THIRTEENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Thirteenth));
	classDefVhc.add_static_constant("GEAR_FOURTEENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Fourteenth));
	classDefVhc.add_static_constant("GEAR_FIFTEENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Fifteenth));
	classDefVhc.add_static_constant("GEAR_SIXTEENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Sixteenth));
	classDefVhc.add_static_constant("GEAR_SEVENTEENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Seventeenth));
	classDefVhc.add_static_constant("GEAR_EIGHTEENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Eighteenth));
	classDefVhc.add_static_constant("GEAR_NINETEENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Nineteenth));
	classDefVhc.add_static_constant("GEAR_TWENTIETH",umath::to_integral(pragma::physics::IVehicle::Gear::Twentieth));
	classDefVhc.add_static_constant("GEAR_TWENTYFIRST",umath::to_integral(pragma::physics::IVehicle::Gear::Twentyfirst));
	classDefVhc.add_static_constant("GEAR_TWENTYSECOND",umath::to_integral(pragma::physics::IVehicle::Gear::Twentysecond));
	classDefVhc.add_static_constant("GEAR_TWENTYTHIRD",umath::to_integral(pragma::physics::IVehicle::Gear::Twentythird));
	classDefVhc.add_static_constant("GEAR_TWENTYFOURTH",umath::to_integral(pragma::physics::IVehicle::Gear::Twentyfourth));
	classDefVhc.add_static_constant("GEAR_TWENTYFIFTH",umath::to_integral(pragma::physics::IVehicle::Gear::Twentyfifth));
	classDefVhc.add_static_constant("GEAR_TWENTYSIXTH",umath::to_integral(pragma::physics::IVehicle::Gear::Twentysixth));
	classDefVhc.add_static_constant("GEAR_TWENTYSEVENTH",umath::to_integral(pragma::physics::IVehicle::Gear::Twentyseventh));
	classDefVhc.add_static_constant("GEAR_TWENTYEIGHTH",umath::to_integral(pragma::physics::IVehicle::Gear::Twentyeighth));
	classDefVhc.add_static_constant("GEAR_TWENTYNINTH",umath::to_integral(pragma::physics::IVehicle::Gear::Twentyninth));
	classDefVhc.add_static_constant("GEAR_THIRTIETH",umath::to_integral(pragma::physics::IVehicle::Gear::Thirtieth));
	classDefVhc.add_static_constant("GEAR_COUNT",umath::to_integral(pragma::physics::IVehicle::Gear::Count));
	classDefVhc.def("IsValid",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		Lua::PushBool(l,hPhys != nullptr);
	}));
	classDefVhc.def("Remove",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->ClaimOwnership().Remove();
	}));
	classDefVhc.def("GetCollisionObject",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		auto *pCollisionObject = hPhys->GetCollisionObject();
		if(pCollisionObject == nullptr)
			return;
		pCollisionObject->Push(l);
	}));
	classDefVhc.def("SetUseDigitalInputs",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,bool)>([](lua_State *l,pragma::physics::IVehicle *hPhys,bool useDigitalInputs) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetUseDigitalInputs(useDigitalInputs);
	}));
	classDefVhc.def("SetBrakeFactor",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,float)>([](lua_State *l,pragma::physics::IVehicle *hPhys,float brakeFactor) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetBrakeFactor(brakeFactor);
	}));
	classDefVhc.def("SetHandBrakeFactor",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,float)>([](lua_State *l,pragma::physics::IVehicle *hPhys,float brakeFactor) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetHandbrakeFactor(brakeFactor);
	}));
	classDefVhc.def("SetAccelerationFactor",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,float)>([](lua_State *l,pragma::physics::IVehicle *hPhys,float accFactor) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetAccelerationFactor(accFactor);
	}));
	classDefVhc.def("SetSteerFactor",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,float)>([](lua_State *l,pragma::physics::IVehicle *hPhys,float turnFactor) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetSteerFactor(turnFactor);
	}));
	classDefVhc.def("SetGear",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,uint32_t)>([](lua_State *l,pragma::physics::IVehicle *hPhys,uint32_t gear) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetGear(static_cast<pragma::physics::IVehicle::Gear>(gear));
	}));
	classDefVhc.def("SetGearDown",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetGearDown();
	}));
	classDefVhc.def("SetGearUp",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetGearUp();
	}));
	classDefVhc.def("SetGearSwitchTime",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,float)>([](lua_State *l,pragma::physics::IVehicle *hPhys,float switchTime) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetGearSwitchTime(switchTime);
	}));
	classDefVhc.def("ChangeToGear",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,uint32_t)>([](lua_State *l,pragma::physics::IVehicle *hPhys,uint32_t gear) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->ChangeToGear(static_cast<pragma::physics::IVehicle::Gear>(gear));
	}));
	classDefVhc.def("SetUseAutoGears",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,bool)>([](lua_State *l,pragma::physics::IVehicle *hPhys,bool useAutoGears) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetUseAutoGears(useAutoGears);
	}));
	classDefVhc.def("ShouldUseAutoGears",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		Lua::PushBool(l,hPhys->ShouldUseAutoGears());
	}));
	classDefVhc.def("GetCurrentGear",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		Lua::PushInt(l,umath::to_integral(hPhys->GetCurrentGear()));
	}));
	classDefVhc.def("GetEngineRotationSpeed",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		Lua::PushNumber(l,hPhys->GetEngineRotationSpeed());
	}));
	classDefVhc.def("SetRestState",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetRestState();
	}));
	classDefVhc.def("ResetControls",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->ResetControls();
	}));
	classDefVhc.def("SetWheelRotationAngle",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,uint32_t,float)>([](lua_State *l,pragma::physics::IVehicle *hPhys,uint32_t wheelIdx,float angle) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetWheelRotationAngle(wheelIdx,angle);
	}));
	classDefVhc.def("SetWheelRotationSpeed",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*,uint32_t,float)>([](lua_State *l,pragma::physics::IVehicle *hPhys,uint32_t wheelIdx,float speed) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		hPhys->SetWheelRotationSpeed(wheelIdx,speed);
	}));
	classDefVhc.def("IsInAir",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		Lua::PushBool(l,hPhys->IsInAir());
	}));
	classDefVhc.def("GetWheelCount",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		Lua::PushInt(l,hPhys->GetWheelCount());
	}));
	classDefVhc.def("GetForwardSpeed",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		Lua::PushNumber(l,hPhys->GetForwardSpeed());
	}));
	classDefVhc.def("GetSidewaysSpeed",static_cast<void(*)(lua_State*,pragma::physics::IVehicle*)>([](lua_State *l,pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l,hPhys) == false)
			return;
		Lua::PushNumber(l,hPhys->GetSidewaysSpeed());
	}));
	physMod[classDefVhc];

	auto classMat = luabind::class_<pragma::physics::IMaterial,pragma::physics::IBase>("Material");
	classMat.def("SetFriction",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&,float)>([](lua_State *l,pragma::physics::IMaterial &mat,float friction) {
		mat.SetFriction(friction);
	}));
	classMat.def("GetStaticFriction",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&)>([](lua_State *l,pragma::physics::IMaterial &mat) {
		Lua::PushNumber(l,mat.GetStaticFriction());
	}));
	classMat.def("SetStaticFriction",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&,float)>([](lua_State *l,pragma::physics::IMaterial &mat,float friction) {
		mat.SetStaticFriction(friction);
	}));
	classMat.def("SetDynamicFriction",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&,float)>([](lua_State *l,pragma::physics::IMaterial &mat,float friction) {
		mat.SetDynamicFriction(friction);
	}));
	classMat.def("GetDynamicFriction",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&)>([](lua_State *l,pragma::physics::IMaterial &mat) {
		Lua::PushNumber(l,mat.GetDynamicFriction());
	}));
	classMat.def("GetRestitution",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&)>([](lua_State *l,pragma::physics::IMaterial &mat) {
		Lua::PushNumber(l,mat.GetRestitution());
	}));
	classMat.def("SetRestitution",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&,float)>([](lua_State *l,pragma::physics::IMaterial &mat,float restitution) {
		mat.SetRestitution(restitution);
	}));
	classMat.def("SetSurfaceMaterial",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&,SurfaceMaterial&)>([](lua_State *l,pragma::physics::IMaterial &mat,SurfaceMaterial &surfMat) {
		mat.SetSurfaceMaterial(surfMat);
	}));
	classMat.def("GetSurfaceMaterial",static_cast<void(*)(lua_State*,pragma::physics::IMaterial&)>([](lua_State *l,pragma::physics::IMaterial &mat) {
		Lua::Push<SurfaceMaterial*>(l,mat.GetSurfaceMaterial());
	}));
	physMod[classMat];

	auto classDefRayCastData = luabind::class_<TraceData>("RayCastData");
	classDefRayCastData.def(luabind::constructor<>());
	classDefRayCastData.def("SetShape",static_cast<void(*)(lua_State*,TraceData&,const pragma::physics::IConvexShape&)>(&Lua_TraceData_SetSource));
	classDefRayCastData.def("SetSource",static_cast<void(TraceData::*)(const Vector3&)>(&TraceData::SetSource));
	classDefRayCastData.def("SetSourceRotation",&TraceData::SetSourceRotation);
	classDefRayCastData.def("SetSource",static_cast<void(TraceData::*)(const umath::Transform&)>(&TraceData::SetSource));
	classDefRayCastData.def("SetTarget",static_cast<void(TraceData::*)(const Vector3&)>(&TraceData::SetTarget));
	classDefRayCastData.def("SetTargetRotation",&TraceData::SetTargetRotation);
	classDefRayCastData.def("SetTarget",static_cast<void(TraceData::*)(const umath::Transform&)>(&TraceData::SetTarget));
	classDefRayCastData.def("SetRotation",&TraceData::SetRotation);
	classDefRayCastData.def("SetFlags",&Lua_TraceData_SetFlags);
	classDefRayCastData.def("SetFilter",&Lua_TraceData_SetFilter);
	classDefRayCastData.def("SetCollisionFilterMask",&Lua_TraceData_SetCollisionFilterMask);
	classDefRayCastData.def("SetCollisionFilterGroup",&Lua_TraceData_SetCollisionFilterGroup);
	classDefRayCastData.def("GetSourceTransform",&Lua_TraceData_GetSourceTransform);
	classDefRayCastData.def("GetTargetTransform",&Lua_TraceData_GetTargetTransform);
	classDefRayCastData.def("GetSourceOrigin",&Lua_TraceData_GetSourceOrigin);
	classDefRayCastData.def("GetTargetOrigin",&Lua_TraceData_GetTargetOrigin);
	classDefRayCastData.def("GetSourceRotation",&Lua_TraceData_GetSourceRotation);
	classDefRayCastData.def("GetTargetRotation",&Lua_TraceData_GetTargetRotation);
	classDefRayCastData.def("GetDistance",&Lua_TraceData_GetDistance);
	classDefRayCastData.def("GetDirection",&Lua_TraceData_GetDirection);
	physMod[classDefRayCastData];

	auto classDefRayCastResult = luabind::class_<TraceResult>("RayCastResult");
	classDefRayCastResult.add_static_constant("HIT_TYPE_BLOCK",umath::to_integral(RayCastHitType::Block));
	classDefRayCastResult.add_static_constant("HIT_TYPE_TOUCH",umath::to_integral(RayCastHitType::Touch));
	classDefRayCastResult.add_static_constant("HIT_TYPE_NONE",umath::to_integral(RayCastHitType::None));
	classDefRayCastResult.def_readonly("hitType",reinterpret_cast<std::underlying_type_t<decltype(TraceResult::hitType)> TraceResult::*>(&TraceResult::hitType));
	classDefRayCastResult.property("entity",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		if(tr.entity.IsValid() == false)
			return;
		lua_pushentity(l,tr.entity);
	}));
	classDefRayCastResult.def_readonly("physObj",&TraceResult::physObj);
	classDefRayCastResult.def_readonly("fraction",&TraceResult::fraction);
	classDefRayCastResult.def_readonly("distance",&TraceResult::distance);
	classDefRayCastResult.def_readonly("normal",&TraceResult::normal);
	classDefRayCastResult.def_readonly("position",&TraceResult::position);
	classDefRayCastResult.def_readonly("startPosition",&TraceResult::startPosition);
	classDefRayCastResult.property("colObj",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		if(tr.collisionObj.IsValid() == false)
			return;
		tr.collisionObj->Push(l);
	}));
	classDefRayCastResult.property("mesh",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
		tr.GetMeshes(&mesh,&subMesh);

		if(mesh == nullptr)
			return;
		Lua::Push<std::shared_ptr<::ModelMesh>>(l,mesh->shared_from_this());
	}));
	classDefRayCastResult.property("subMesh",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
		tr.GetMeshes(&mesh,&subMesh);

		if(subMesh == nullptr)
			return;
		Lua::Push<std::shared_ptr<::ModelSubMesh>>(l,subMesh->shared_from_this());
	}));
	classDefRayCastResult.property("material",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		auto *mat = tr.GetMaterial();
		if(mat == nullptr)
			return;
		Lua::Push<Material*>(l,mat);
	}));
	classDefRayCastResult.property("materialName",static_cast<void(*)(lua_State*,TraceResult&)>([](lua_State *l,TraceResult &tr) {
		std::string mat;
		if(tr.GetMaterial(mat) == false)
			return;
		Lua::PushString(l,mat);
	}));
	physMod[classDefRayCastResult];

	auto classDefChassisCreateInfo = luabind::class_<pragma::physics::ChassisCreateInfo>("ChassisCreateInfo");
	classDefChassisCreateInfo.def(luabind::constructor<>());
	classDefChassisCreateInfo.property("momentOfInertia",static_cast<void(*)(lua_State*,pragma::physics::ChassisCreateInfo&)>([](lua_State *l,pragma::physics::ChassisCreateInfo &chassisCreateInfo) {
		if(chassisCreateInfo.momentOfInertia.has_value())
			Lua::Push<Vector3>(l,*chassisCreateInfo.momentOfInertia);
	}),static_cast<void(*)(lua_State*,pragma::physics::ChassisCreateInfo&,const Vector3&)>([](lua_State *l,pragma::physics::ChassisCreateInfo &chassisCreateInfo,const Vector3 &moi) {
		chassisCreateInfo.momentOfInertia = moi;
	}));
	classDefChassisCreateInfo.property("centerOfMass",static_cast<void(*)(lua_State*,pragma::physics::ChassisCreateInfo&)>([](lua_State *l,pragma::physics::ChassisCreateInfo &chassisCreateInfo) {
		if(chassisCreateInfo.centerOfMass.has_value())
			Lua::Push<Vector3>(l,*chassisCreateInfo.centerOfMass);
	}),static_cast<void(*)(lua_State*,pragma::physics::ChassisCreateInfo&,const Vector3&)>([](lua_State *l,pragma::physics::ChassisCreateInfo &chassisCreateInfo,const Vector3 &com) {
			chassisCreateInfo.centerOfMass = com;
	}));
	classDefChassisCreateInfo.def("AddShapeIndex",static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&,uint32_t)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcCreateInfo,uint32_t shapeIndex) {
		vhcCreateInfo.chassis.shapeIndices.push_back(shapeIndex);
	}));
	classDefChassisCreateInfo.def("GetShapeIndices",static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcCreateInfo) {
		auto t = Lua::CreateTable(l);
		auto i = 1;
		for(auto idx : vhcCreateInfo.chassis.shapeIndices)
		{
			Lua::PushInt(l,i++);
			Lua::PushInt(l,idx);
			Lua::SetTableValue(l,t);
		}
	}));
	physMod[classDefChassisCreateInfo];

	auto classDefSuspensionInfo = luabind::class_<pragma::physics::WheelCreateInfo::SuspensionInfo>("SuspensionInfo");
	classDefSuspensionInfo.def(luabind::constructor<>());
	classDefSuspensionInfo.def_readwrite("maxCompression",&pragma::physics::WheelCreateInfo::SuspensionInfo::maxCompression);
	classDefSuspensionInfo.def_readwrite("maxDroop",&pragma::physics::WheelCreateInfo::SuspensionInfo::maxDroop);
	classDefSuspensionInfo.def_readwrite("springStrength",&pragma::physics::WheelCreateInfo::SuspensionInfo::springStrength);
	classDefSuspensionInfo.def_readwrite("springDamperRate",&pragma::physics::WheelCreateInfo::SuspensionInfo::springDamperRate);
	classDefSuspensionInfo.def_readwrite("camberAngleAtRest",&pragma::physics::WheelCreateInfo::SuspensionInfo::camberAngleAtRest);
	classDefSuspensionInfo.def_readwrite("camberAngleAtMaxDroop",&pragma::physics::WheelCreateInfo::SuspensionInfo::camberAngleAtMaxDroop);
	classDefSuspensionInfo.def_readwrite("camberAngleAtMaxCompression",&pragma::physics::WheelCreateInfo::SuspensionInfo::camberAngleAtMaxCompression);

	auto classDefVhcCreateInfo = luabind::class_<pragma::physics::VehicleCreateInfo>("VehicleCreateInfo");
	classDefVhcCreateInfo.scope[luabind::def("CreateStandardFourWheelDrive",static_cast<void(*)(lua_State*,luabind::object,float,float)>([](lua_State *l,luabind::object oWheelCenterOffsets,float handBrakeTorque,float maxSteeringAngle) {
		create_standard_four_wheel_drive(l,oWheelCenterOffsets,handBrakeTorque,maxSteeringAngle);
	}))];
	classDefVhcCreateInfo.scope[luabind::def("CreateStandardFourWheelDrive",static_cast<void(*)(lua_State*,luabind::object,float)>([](lua_State *l,luabind::object oWheelCenterOffsets,float handBrakeTorque) {
		create_standard_four_wheel_drive(l,oWheelCenterOffsets,handBrakeTorque);
	}))];
	classDefVhcCreateInfo.scope[luabind::def("CreateStandardFourWheelDrive",static_cast<void(*)(lua_State*,luabind::object)>([](lua_State *l,luabind::object oWheelCenterOffsets) {
		create_standard_four_wheel_drive(l,oWheelCenterOffsets);
	}))];
	classDefVhcCreateInfo.add_static_constant("WHEEL_DRIVE_FRONT",umath::to_integral(pragma::physics::VehicleCreateInfo::WheelDrive::Front));
	classDefVhcCreateInfo.add_static_constant("WHEEL_DRIVE_REAR",umath::to_integral(pragma::physics::VehicleCreateInfo::WheelDrive::Rear));
	classDefVhcCreateInfo.add_static_constant("WHEEL_DRIVE_FOUR",umath::to_integral(pragma::physics::VehicleCreateInfo::WheelDrive::Four));
	classDefVhcCreateInfo.def(luabind::constructor<>());
	classDefVhcCreateInfo.property("actor",static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcDesc) {
		if(vhcDesc.actor.IsValid())
			vhcDesc.actor->Push(l);
	}),static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&,util::TSharedHandle<pragma::physics::IRigidBody>&)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcDesc,util::TSharedHandle<pragma::physics::IRigidBody> &actor) {
		vhcDesc.actor = actor;
	}));
	classDefVhcCreateInfo.def_readwrite("chassis",&pragma::physics::VehicleCreateInfo::chassis);
	classDefVhcCreateInfo.def_readwrite("maxEngineTorque",&pragma::physics::VehicleCreateInfo::maxEngineTorque);
	classDefVhcCreateInfo.def_readwrite("maxEngineRotationSpeed",&pragma::physics::VehicleCreateInfo::maxEngineRotationSpeed);
	classDefVhcCreateInfo.def_readwrite("gearSwitchTime",&pragma::physics::VehicleCreateInfo::gearSwitchTime);
	classDefVhcCreateInfo.def_readwrite("clutchStrength",&pragma::physics::VehicleCreateInfo::clutchStrength);
	classDefVhcCreateInfo.def_readwrite("gravityFactor",&pragma::physics::VehicleCreateInfo::gravityFactor);
	classDefVhcCreateInfo.def_readwrite("wheelDrive",reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::VehicleCreateInfo::wheelDrive)> pragma::physics::VehicleCreateInfo::*>(&pragma::physics::VehicleCreateInfo::wheelDrive));
	classDefVhcCreateInfo.def("AddWheel",static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&,pragma::physics::WheelCreateInfo&)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcCreateInfo,pragma::physics::WheelCreateInfo &wheelCreateInfo) {
		auto wheelType = umath::to_integral(pragma::physics::VehicleCreateInfo::GetWheelType(wheelCreateInfo));
		auto it = std::find_if(vhcCreateInfo.wheels.begin(),vhcCreateInfo.wheels.end(),[wheelType](const pragma::physics::WheelCreateInfo &wheelDescOther) {
			return wheelType < umath::to_integral(pragma::physics::VehicleCreateInfo::GetWheelType(wheelDescOther));
		});
		// Wheels have to be inserted in the correct order! (FrontLeft -> FrontRight -> RearLeft -> RearRight)
		vhcCreateInfo.wheels.insert(it,wheelCreateInfo);
	}));
	classDefVhcCreateInfo.def("GetWheels",static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcCreateInfo) {
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &wheel : vhcCreateInfo.wheels)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::physics::WheelCreateInfo*>(l,&wheel);
			Lua::SetTableValue(l,t);
		}
	}));
	classDefVhcCreateInfo.def("AddAntiRollBar",static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&,pragma::physics::VehicleCreateInfo::AntiRollBar&)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcCreateInfo,pragma::physics::VehicleCreateInfo::AntiRollBar &antiRollBar) {
		vhcCreateInfo.antiRollBars.push_back(antiRollBar);
	}));
	classDefVhcCreateInfo.def("GetAntiRollBars",static_cast<void(*)(lua_State*,pragma::physics::VehicleCreateInfo&)>([](lua_State *l,pragma::physics::VehicleCreateInfo &vhcCreateInfo) {
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &antiRollBar : vhcCreateInfo.antiRollBars)
		{
			Lua::PushInt(l,idx++);
			Lua::Push<pragma::physics::VehicleCreateInfo::AntiRollBar*>(l,&antiRollBar);
			Lua::SetTableValue(l,t);
		}
	}));
	classDefVhcCreateInfo.scope[classDefSuspensionInfo];
	physMod[classDefVhcCreateInfo];

	auto classDefAntiRollBar = luabind::class_<pragma::physics::VehicleCreateInfo::AntiRollBar>("AntiRollBar");
	classDefAntiRollBar.def(luabind::constructor<>());
	classDefAntiRollBar.def_readwrite("wheel0",reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::VehicleCreateInfo::AntiRollBar::wheel0)> pragma::physics::VehicleCreateInfo::AntiRollBar::*>(&pragma::physics::VehicleCreateInfo::AntiRollBar::wheel0));
	classDefAntiRollBar.def_readwrite("wheel1",reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::VehicleCreateInfo::AntiRollBar::wheel1)> pragma::physics::VehicleCreateInfo::AntiRollBar::*>(&pragma::physics::VehicleCreateInfo::AntiRollBar::wheel1));
	classDefAntiRollBar.def_readwrite("stiffness",&pragma::physics::VehicleCreateInfo::AntiRollBar::stiffness);
	classDefVhcCreateInfo.scope[classDefAntiRollBar];
	physMod[classDefAntiRollBar];

	auto classDefWheelCreateInfo = luabind::class_<pragma::physics::WheelCreateInfo>("WheelCreateInfo");
	classDefWheelCreateInfo.scope[luabind::def("CreateStandardFrontWheel",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		Lua::Push<pragma::physics::WheelCreateInfo>(l,pragma::physics::WheelCreateInfo::CreateStandardFrontWheel());
	}))];
	classDefWheelCreateInfo.scope[luabind::def("CreateStandardRearWheel",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		Lua::Push<pragma::physics::WheelCreateInfo>(l,pragma::physics::WheelCreateInfo::CreateStandardRearWheel());
	}))];
	classDefWheelCreateInfo.add_static_constant("FLAG_NONE",umath::to_integral(pragma::physics::WheelCreateInfo::Flags::None));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_FRONT",umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Front));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_REAR",umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Rear));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_LEFT",umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Left));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_RIGHT",umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Right));
	classDefWheelCreateInfo.def(luabind::constructor<>());
	classDefWheelCreateInfo.def_readwrite("suspension",&pragma::physics::WheelCreateInfo::suspension);
	classDefWheelCreateInfo.def_readwrite("width",&pragma::physics::WheelCreateInfo::width);
	classDefWheelCreateInfo.def_readwrite("radius",&pragma::physics::WheelCreateInfo::radius);
	classDefWheelCreateInfo.def_readwrite("shapeIndex",&pragma::physics::WheelCreateInfo::shapeIndex);
	classDefWheelCreateInfo.def_readwrite("tireType",&pragma::physics::WheelCreateInfo::tireType);
	classDefWheelCreateInfo.def_readwrite("flags",reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::WheelCreateInfo::flags)> pragma::physics::WheelCreateInfo::*>(&pragma::physics::WheelCreateInfo::flags));
	classDefWheelCreateInfo.property("momentOfInertia",static_cast<void(*)(lua_State*,pragma::physics::WheelCreateInfo&)>([](lua_State *l,pragma::physics::WheelCreateInfo &wheelCreateInfo) {
		if(wheelCreateInfo.momentOfInertia.has_value())
			Lua::PushNumber(l,*wheelCreateInfo.momentOfInertia);
	}),static_cast<void(*)(lua_State*,pragma::physics::WheelCreateInfo&,float)>([](lua_State *l,pragma::physics::WheelCreateInfo &wheelCreateInfo,float moi) {
		wheelCreateInfo.momentOfInertia = moi;
	}));
	classDefWheelCreateInfo.def_readwrite("chassisOffset",&pragma::physics::WheelCreateInfo::chassisOffset);
	classDefWheelCreateInfo.def_readwrite("maxHandBrakeTorque",&pragma::physics::WheelCreateInfo::maxHandbrakeTorque);
	classDefWheelCreateInfo.def_readwrite("maxSteeringAngle",&pragma::physics::WheelCreateInfo::maxSteeringAngle);
	physMod[classDefWheelCreateInfo];

	auto classDefTransform = luabind::class_<umath::Transform>("Transform");
	classDefTransform.def(luabind::constructor<const Mat4&>());
	classDefTransform.def(luabind::constructor<const Vector3&,const Quat&>());
	classDefTransform.def(luabind::constructor<>());
	classDefTransform.def(luabind::tostring(luabind::self));
	classDefTransform.def("Copy",static_cast<void(*)(lua_State*,umath::Transform&)>([](lua_State *l,umath::Transform &t) {
		Lua::Push<umath::Transform>(l,t);
	}));
	classDefTransform.def("Set",static_cast<void(*)(lua_State*,umath::Transform&,const umath::Transform&)>([](lua_State *l,umath::Transform &t,const umath::Transform &tOther) {
		t = tOther;
	}));
	classDefTransform.def("GetOrigin",static_cast<void(*)(lua_State*,umath::Transform&)>([](lua_State *l,umath::Transform &t) {
		Lua::Push<Vector3>(l,t.GetOrigin());
	}));
	classDefTransform.def("GetRotation",static_cast<void(*)(lua_State*,umath::Transform&)>([](lua_State *l,umath::Transform &t) {
		Lua::Push<Quat>(l,t.GetRotation());
	}));
	classDefTransform.def("SetOrigin",static_cast<void(*)(lua_State*,umath::Transform&,const Vector3&)>([](lua_State *l,umath::Transform &t,const Vector3 &origin) {
		t.SetOrigin(origin);
	}));
	classDefTransform.def("SetRotation",static_cast<void(*)(lua_State*,umath::Transform&,const Quat&)>([](lua_State *l,umath::Transform &t,const Quat &rotation) {
		t.SetRotation(rotation);
	}));
	classDefTransform.def("SetIdentity",static_cast<void(*)(lua_State*,umath::Transform&)>([](lua_State *l,umath::Transform &t) {
		t.SetIdentity();
	}));
	classDefTransform.def("TranslateGlobal",static_cast<void(*)(lua_State*,umath::Transform&,const Vector3&)>([](lua_State *l,umath::Transform &t,const Vector3 &v) {
		t.TranslateGlobal(v);
	}));
	classDefTransform.def("TranslateLocal",static_cast<void(*)(lua_State*,umath::Transform&,const Vector3&)>([](lua_State *l,umath::Transform &t,const Vector3 &v) {
		t.TranslateLocal(v);
	}));
	classDefTransform.def("RotateGlobal",static_cast<void(*)(lua_State*,umath::Transform&,const Quat&)>([](lua_State *l,umath::Transform &t,const Quat &rot) {
		t.RotateGlobal(rot);
	}));
	classDefTransform.def("RotateLocal",static_cast<void(*)(lua_State*,umath::Transform&,const Quat&)>([](lua_State *l,umath::Transform &t,const Quat &rot) {
		t.RotateLocal(rot);
	}));
	classDefTransform.def("TransformGlobal",static_cast<void(*)(lua_State*,umath::Transform&,const umath::Transform&)>([](lua_State *l,umath::Transform &t,const umath::Transform &t2) {
		t = t2 *t;
	}));
	classDefTransform.def("TransformLocal",static_cast<void(*)(lua_State*,umath::Transform&,const umath::Transform&)>([](lua_State *l,umath::Transform &t,const umath::Transform &t2) {
		t *= t2;
	}));
	classDefTransform.def("GetInverse",static_cast<void(*)(lua_State*,umath::Transform&)>([](lua_State *l,umath::Transform &t) {
		Lua::Push<umath::Transform>(l,t.GetInverse());
	}));
	classDefTransform.def("ToMatrix",static_cast<void(*)(lua_State*,umath::Transform&)>([](lua_State *l,umath::Transform &t) {
		Lua::Push<Mat4>(l,t.ToMatrix());
	}));
	classDefTransform.def("SetMatrix",static_cast<void(*)(lua_State*,umath::Transform&,const Mat4&)>([](lua_State *l,umath::Transform &t,const Mat4 &m) {
		Mat4 transformation;
		Vector3 scale;
		Quat rotation;
		Vector3 translation;
		Vector3 skew;
		Vector4 perspective;
		glm::decompose(transformation,scale,rotation,translation,skew,perspective);
		t.SetOrigin(translation);
		t.SetRotation(rotation);
	}));
	classDefTransform.def("Interpolate",static_cast<void(*)(lua_State*,umath::Transform&,umath::Transform&,float)>([](lua_State *l,umath::Transform &t,umath::Transform &dst,float factor) {
		t.Interpolate(dst,factor);
	}));
	classDefTransform.def("InterpolateToIdentity",static_cast<void(*)(lua_State*,umath::Transform&,float)>([](lua_State *l,umath::Transform &t,float factor) {
		t.InterpolateToIdentity(factor);
	}));
	classDefTransform.def(luabind::const_self *luabind::const_self);
	classDefTransform.def(luabind::const_self *Vector3());
	classDefTransform.def(luabind::const_self *Quat());
	physMod[classDefTransform];

	auto classDefScaledTransform = luabind::class_<umath::ScaledTransform,umath::Transform>("ScaledTransform");
	classDefScaledTransform.def(luabind::constructor<const Mat4&>());
	classDefScaledTransform.def(luabind::constructor<const Vector3&,const Quat&>());
	classDefScaledTransform.def(luabind::constructor<const Vector3&,const Quat&,const Vector3&>());
	classDefScaledTransform.def(luabind::constructor<>());
	classDefScaledTransform.def(luabind::tostring(luabind::self));
	classDefScaledTransform.def("Copy",static_cast<void(*)(lua_State*,umath::ScaledTransform&)>([](lua_State *l,umath::ScaledTransform &t) {
		Lua::Push<umath::ScaledTransform>(l,t);
	}));
	classDefScaledTransform.def("Set",static_cast<void(*)(lua_State*,umath::ScaledTransform&,const umath::ScaledTransform&)>([](lua_State *l,umath::ScaledTransform &t,const umath::ScaledTransform &tOther) {
		t = tOther;
	}));
	classDefScaledTransform.def("GetScale",static_cast<void(*)(lua_State*,umath::ScaledTransform&)>([](lua_State *l,umath::ScaledTransform &t) {
		Lua::Push<Vector3>(l,t.GetScale());
	}));
	classDefScaledTransform.def("SetScale",static_cast<void(*)(lua_State*,umath::ScaledTransform&,const Vector3&)>([](lua_State *l,umath::ScaledTransform &t,const Vector3 &scale) {
		t.SetScale(scale);
	}));
	classDefScaledTransform.def("Scale",static_cast<void(*)(lua_State*,umath::ScaledTransform&,const Vector3&)>([](lua_State *l,umath::ScaledTransform &t,const Vector3 &scale) {
		t.Scale(scale);
	}));
	classDefScaledTransform.def("GetInverse",static_cast<void(*)(lua_State*,umath::ScaledTransform&)>([](lua_State *l,umath::ScaledTransform &t) {
		Lua::Push<umath::ScaledTransform>(l,t.GetInverse());
	}));
	classDefScaledTransform.def("Interpolate",static_cast<void(*)(lua_State*,umath::ScaledTransform&,umath::ScaledTransform&,float)>([](lua_State *l,umath::ScaledTransform &t,umath::ScaledTransform &dst,float factor) {
		t.Interpolate(dst,factor);
	}));
	classDefScaledTransform.def("InterpolateToIdentity",static_cast<void(*)(lua_State*,umath::ScaledTransform&,float)>([](lua_State *l,umath::ScaledTransform &t,float factor) {
		t.InterpolateToIdentity(factor);
	}));
	classDefScaledTransform.def(luabind::const_self *umath::Transform());
	classDefScaledTransform.def(luabind::const_self *luabind::const_self);
	classDefScaledTransform.def(luabind::const_self *Vector3());
	classDefScaledTransform.def(luabind::const_self *Quat());
	physMod[classDefScaledTransform];

	auto classTreeIkTree = luabind::class_<Tree>("IKTree");
	classTreeIkTree.scope[luabind::def("Create",static_cast<void(*)(lua_State*)>([](lua_State *l) {
		auto tree = std::make_shared<Tree>();
		Lua::Push<std::shared_ptr<Tree>>(l,tree);
	}))];
#ifdef ENABLE_DEPRECATED_PHYSICS
	classTreeIkTree.def("Draw",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		auto *game = engine->GetNetworkState(l)->GetGameState();
		auto fGetLocalTransform = [](const Node* node, btTransform& act) {
			btVector3 axis = btVector3(node->v.x, node->v.y, node->v.z);
			btQuaternion rot(0, 0, 0, 1);
			if (axis.length())
			{
				rot = btQuaternion (axis, node->theta);
			}
			act.setIdentity();
			act.setRotation(rot);
			act.setOrigin(btVector3(node->r.x, node->r.y, node->r.z));
		};
		std::function<void(Node*,const btTransform&)> fDrawTree = nullptr;
		fDrawTree = [&fGetLocalTransform,&fDrawTree,game](Node* node,const btTransform& tr) {
			btVector3 lineColor = btVector3(0, 0, 0);
			int lineWidth = 2;
			auto fUpdateLine = [game](int32_t tIdx,const Vector3 &start,const Vector3 &end,const Color &col) {
				/*auto it = m_dbgObjects.find(tIdx);
				if(it == m_dbgObjects.end())
					it = m_dbgObjects.insert(std::make_pair(tIdx,DebugRenderer::DrawLine(start,end,col))).first;
				auto &wo = static_cast<DebugRenderer::WorldObject&>(*m_dbgObjects.at(tIdx));
				wo.GetVertices().at(0) = start;
				wo.GetVertices().at(1) = end;
				wo.UpdateVertexBuffer();*/
				game->DrawLine(start,end,col,0.05f);
				//DebugRenderer::DrawLine(start,end,col,0.05f);
			};
			if (node != 0) {
			//	glPushMatrix();
				btVector3 pos = btVector3(tr.getOrigin().x(), tr.getOrigin().y(), tr.getOrigin().z());
				btVector3 color = btVector3(0, 1, 0);
				int pointSize = 10;
				auto enPos = uvec::create(pos);
				//auto it = m_dbgObjects.find(0u);
				//if(it == m_dbgObjects.end())
				//	it = m_dbgObjects.insert(std::make_pair(0u,DebugRenderer::DrawPoint(enPos,Color::Lime))).first;
				//it->second->SetPos(enPos);

				auto enForward = uvec::create((tr.getBasis().getColumn(0)));
				auto enRight = uvec::create((tr.getBasis().getColumn(1)));
				auto enUp = uvec::create((tr.getBasis().getColumn(2)));
				fUpdateLine(1,enPos,enPos +enForward *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Red);
				fUpdateLine(2,enPos,enPos +enRight *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Lime);
				fUpdateLine(3,enPos,enPos +enUp *static_cast<float>(0.05f /PhysEnv::WORLD_SCALE),Color::Aqua);
			
				btVector3 axisLocal = btVector3(node->v.x, node->v.y, node->v.z);
				btVector3 axisWorld = tr.getBasis()*axisLocal;

				fUpdateLine(4,enPos,enPos +0.1f *uvec::create(axisWorld),Color::Yellow);

				//node->DrawNode(node == root);	// Recursively draw node and update ModelView matrix
				if (node->left) {
					btTransform act;
					fGetLocalTransform(node->left, act);
				
					btTransform trl = tr*act;
					auto trOrigin = uvec::create(tr.getOrigin() /PhysEnv::WORLD_SCALE);
					auto trlOrigin = uvec::create(trl.getOrigin() /PhysEnv::WORLD_SCALE);
					fUpdateLine(5,trOrigin,trlOrigin,Color::Maroon);
					fDrawTree(node->left, trl);		// Draw tree of children recursively
				}
			//	glPopMatrix();
				if (node->right) {
					btTransform act;
					fGetLocalTransform(node->right, act);
					btTransform trr = tr*act;
					auto trOrigin = uvec::create(tr.getOrigin() /PhysEnv::WORLD_SCALE);
					auto trrOrigin = uvec::create(trr.getOrigin() /PhysEnv::WORLD_SCALE);
					fUpdateLine(6,trOrigin,trrOrigin,Color::Silver);
					fDrawTree(node->right,trr);		// Draw right siblings recursively
				}
			}
		};
		auto fRenderScene = [&fGetLocalTransform,&fDrawTree](Tree &tree) {
			btTransform act;
			fGetLocalTransform(tree.GetRoot(), act);
			fDrawTree(tree.GetRoot(), act);
		
			//btVector3 pos = btVector3(targetaa[0].x, targetaa[0].y, targetaa[0].z);
			//btQuaternion orn(0, 0, 0, 1);
		};
		fRenderScene(tree);
	}));
#endif
	classTreeIkTree.def("GetNodeCount",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		Lua::PushInt(l,tree.GetNumNode());
	}));
	classTreeIkTree.def("GetEffectorCount",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		Lua::PushInt(l,tree.GetNumEffector());
	}));
	classTreeIkTree.def("GetJointCount",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		Lua::PushInt(l,tree.GetNumJoint());
	}));
	classTreeIkTree.def("Compute",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.Compute();
	}));
	classTreeIkTree.def("Init",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.Init();
	}));
	classTreeIkTree.def("UnFreeze",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.UnFreeze();
	}));
	classTreeIkTree.def("Print",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		tree.Print();
	}));
	classTreeIkTree.def("InsertRoot",static_cast<void(*)(lua_State*,Tree&,Node&)>([](lua_State *l,Tree &tree,Node &node) {
		tree.InsertRoot(&node);
	}));
	classTreeIkTree.def("InsertLeftChild",static_cast<void(*)(lua_State*,Tree&,Node&,Node&)>([](lua_State *l,Tree &tree,Node &parent,Node &child) {
		tree.InsertLeftChild(&parent,&child);
	}));
	classTreeIkTree.def("InsertRightSibling",static_cast<void(*)(lua_State*,Tree&,Node&,Node&)>([](lua_State *l,Tree &tree,Node &parent,Node &child) {
		tree.InsertRightSibling(&parent,&child);
	}));
	classTreeIkTree.def("GetJoint",static_cast<void(*)(lua_State*,Tree&,uint32_t)>([](lua_State *l,Tree &tree,uint32_t nodeIdx) {
		auto *node = tree.GetJoint(nodeIdx);
		if(node == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,node->shared_from_this());
	}));
	classTreeIkTree.def("GetEffector",static_cast<void(*)(lua_State*,Tree&,uint32_t)>([](lua_State *l,Tree &tree,uint32_t nodeIdx) {
		auto *node = tree.GetEffector(nodeIdx);
		if(node == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,node->shared_from_this());
	}));
	classTreeIkTree.def("GetEffectorPosition",static_cast<void(*)(lua_State*,Tree&,uint32_t)>([](lua_State *l,Tree &tree,uint32_t nodeIdx) {
		auto &pos = tree.GetEffectorPosition(nodeIdx);
		Lua::Push<Vector3>(l,Vector3(pos.x,pos.y,pos.z));
	}));
	classTreeIkTree.def("GetRoot",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		auto *root = tree.GetRoot();
		if(root == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,root->shared_from_this());
	}));
	classTreeIkTree.def("GetSuccessor",static_cast<void(*)(lua_State*,Tree&,Node&)>([](lua_State *l,Tree &tree,Node &node) {
		auto *successor = tree.GetSuccessor(&node);
		if(successor == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,successor->shared_from_this());
	}));
	classTreeIkTree.def("GetParent",static_cast<void(*)(lua_State*,Tree&,Node&)>([](lua_State *l,Tree &tree,Node &node) {
		auto *parent = tree.GetParent(&node);
		if(parent == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,parent->shared_from_this());
	}));

	auto classTreeIkTreeNode = luabind::class_<Node>("Node");
	classTreeIkTreeNode.add_static_constant("PURPOSE_JOINT",JOINT);
	classTreeIkTreeNode.add_static_constant("PURPOSE_EFFECTOR",EFFECTOR);
	classTreeIkTreeNode.scope[luabind::def("Create",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,uint32_t,double,double,double)>([](lua_State *l,const Vector3 &origin,const Vector3 &rotAxis,uint32_t purpose,double minTheta,double maxTheta,double restAngle) {
		auto node = std::make_shared<Node>(VectorR3(origin.x,origin.y,origin.z),VectorR3(rotAxis.x,rotAxis.y,rotAxis.z),0.0,static_cast<Purpose>(purpose),minTheta,maxTheta,restAngle);
		Lua::Push<std::shared_ptr<Node>>(l,node);
	}))];
	classTreeIkTreeNode.scope[luabind::def("Create",static_cast<void(*)(lua_State*,const Vector3&,const Vector3&,uint32_t)>([](lua_State *l,const Vector3 &origin,const Vector3 &rotAxis,uint32_t purpose) {
		auto node = std::make_shared<Node>(VectorR3(origin.x,origin.y,origin.z),VectorR3(rotAxis.x,rotAxis.y,rotAxis.z),0.0,static_cast<Purpose>(purpose));
		Lua::Push<std::shared_ptr<Node>>(l,node);
	}))];
	classTreeIkTreeNode.def("GetLocalTransform",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		umath::Transform t {};
		util::ik::get_local_transform(node,t);
		Lua::Push<umath::Transform>(l,t);
	}));
	classTreeIkTreeNode.def("PrintNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.PrintNode();
	}));
	classTreeIkTreeNode.def("GetRotationAxis",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &v = node.v;
		Lua::Push<Vector3>(l,Vector3(v.x,v.y,v.z));
	}));
	classTreeIkTreeNode.def("SetRotationAxis",static_cast<void(*)(lua_State*,Node&,const Vector3&)>([](lua_State *l,Node &node,const Vector3 &axis) {
		auto &v = node.v;
		v = VectorR3(axis.x,axis.y,axis.z);
	}));
	classTreeIkTreeNode.def("GetLeftChildNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto *left = node.left;
		if(left == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,left->shared_from_this());
	}));
	classTreeIkTreeNode.def("GetRightChildNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto *right = node.right;
		if(right == nullptr)
			return;
		Lua::Push<std::shared_ptr<Node>>(l,right->shared_from_this());
	}));
	classTreeIkTreeNode.def("InitNode",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.InitNode();
	}));
	classTreeIkTreeNode.def("GetAttach",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &r = node.GetAttach();
		Lua::Push<Vector3>(l,Vector3(r.x,r.y,r.z));
	}));
	classTreeIkTreeNode.def("SetAttach",static_cast<void(*)(lua_State*,Node&,const Vector3&)>([](lua_State *l,Node &node,const Vector3 &attach) {
		node.attach = VectorR3(attach.x,attach.y,attach.z);
	}));
	classTreeIkTreeNode.def("GetRelativePosition",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &r = node.r;
		Lua::Push<Vector3>(l,Vector3(r.x,r.y,r.z));
	}));
	classTreeIkTreeNode.def("GetTheta",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto theta = node.GetTheta();
		Lua::PushNumber(l,theta);
	}));
	classTreeIkTreeNode.def("AddToTheta",static_cast<void(*)(lua_State*,Node&,double)>([](lua_State *l,Node &node,double delta) {
		auto r = node.AddToTheta(delta);
		Lua::PushNumber(l,r);
	}));
	classTreeIkTreeNode.def("UpdateTheta",static_cast<void(*)(lua_State*,Node&,double)>([](lua_State *l,Node &node,double delta) {
		auto r = node.UpdateTheta(delta);
		Lua::PushNumber(l,r);
	}));
	classTreeIkTreeNode.def("GetS",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &s = node.GetS();
		Lua::Push<Vector3>(l,Vector3(s.x,s.y,s.z));
	}));
	classTreeIkTreeNode.def("GetW",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto &w = node.GetW();
		Lua::Push<Vector3>(l,Vector3(w.x,w.y,w.z));
	}));
	classTreeIkTreeNode.def("GetMinTheta",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto minTheta = node.GetMinTheta();
		Lua::PushNumber(l,minTheta);
	}));
	classTreeIkTreeNode.def("GetMaxTheta",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto maxTheta = node.GetMaxTheta();
		Lua::PushNumber(l,maxTheta);
	}));
	classTreeIkTreeNode.def("GetRestAngle",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		auto restAngle = node.GetRestAngle();
		Lua::PushNumber(l,restAngle);
	}));
	classTreeIkTreeNode.def("SetTheta",static_cast<void(*)(lua_State*,Node&,double)>([](lua_State *l,Node &node,double theta) {
		node.SetTheta(theta);
	}));
	classTreeIkTreeNode.def("ComputeS",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.ComputeS();
	}));
	classTreeIkTreeNode.def("ComputeW",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.ComputeW();
	}));
	classTreeIkTreeNode.def("IsEffector",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushBool(l,node.IsEffector());
	}));
	classTreeIkTreeNode.def("IsJoint",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushBool(l,node.IsJoint());
	}));
	classTreeIkTreeNode.def("GetEffectorIndex",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushInt(l,node.GetEffectorNum());
	}));
	classTreeIkTreeNode.def("GetJointIndex",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushInt(l,node.GetJointNum());
	}));
	classTreeIkTreeNode.def("IsFrozen",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		Lua::PushBool(l,node.IsFrozen());
	}));
	classTreeIkTreeNode.def("Freeze",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.Freeze();
	}));
	classTreeIkTreeNode.def("UnFreeze",static_cast<void(*)(lua_State*,Node&)>([](lua_State *l,Node &node) {
		node.UnFreeze();
	}));

	auto classIkJacobian = luabind::class_<Jacobian>("IKJacobian");
	classIkJacobian.scope[luabind::def("Create",static_cast<void(*)(lua_State*,Tree&)>([](lua_State *l,Tree &tree) {
		auto jacobian = std::make_shared<Jacobian>(&tree);
		Lua::Push<std::shared_ptr<Jacobian>>(l,jacobian);
	}))];
	classIkJacobian.def("ComputeJacobian",static_cast<void(*)(lua_State*,Jacobian&,luabind::object)>([](lua_State *l,Jacobian &jacobian,luabind::object o) {
		Lua::CheckTable(l,2);
		auto numTargets = Lua::GetObjectLength(l,2);
		std::vector<VectorR3> targets;
		targets.reserve(numTargets);
		for(auto i=decltype(numTargets){0};i<numTargets;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,2);
			auto *v = Lua::CheckVector(l,-1);
			targets.push_back(VectorR3(v->x,v->y,v->z));
			Lua::Pop(l,1);
		}
		jacobian.ComputeJacobian(targets.data());
	}));
	classIkJacobian.def("SetJendActive",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.SetJendActive();
	}));
	classIkJacobian.def("SetJtargetActive",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.SetJtargetActive();
	}));
	//classIkJacobian.def("SetJendTrans",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.SetJendTrans();
	//}));
	//classIkJacobian.def("SetDeltaS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian,const Vector3 &s) {
	//	jacobian.SetDeltaS();
	//}));
	classIkJacobian.def("CalcDeltaThetas",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetas();
	}));
	classIkJacobian.def("ZeroDeltaThetas",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.ZeroDeltaThetas();
	}));
	classIkJacobian.def("CalcDeltaThetasTranspose",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasTranspose();
	}));
	classIkJacobian.def("CalcDeltaThetasPseudoinverse",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasPseudoinverse();
	}));
	classIkJacobian.def("CalcDeltaThetasDLS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasDLS();
	}));
	//classIkJacobian.def("CalcDeltaThetasDLS2",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.CalcDeltaThetasDLS2();
	//}));
	classIkJacobian.def("CalcDeltaThetasDLSwithSVD",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasDLSwithSVD();
	}));
	classIkJacobian.def("CalcDeltaThetasSDLS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CalcDeltaThetasSDLS();
	}));
	//classIkJacobian.def("CalcDeltaThetasDLSwithNullspace",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.CalcDeltaThetasDLSwithNullspace();
	//}));
	classIkJacobian.def("UpdateThetas",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.UpdateThetas();
	}));
	classIkJacobian.def("UpdateThetaDot",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.UpdateThetaDot();
	}));
	//classIkJacobian.def("UpdateErrorArray",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.UpdateErrorArray();
	//}));
	//classIkJacobian.def("GetErrorArray",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.GetErrorArray();
	//}));
	classIkJacobian.def("UpdatedSClampValue",static_cast<void(*)(lua_State*,Jacobian&,luabind::object)>([](lua_State *l,Jacobian &jacobian,luabind::object o) {
		Lua::CheckTable(l,2);
		auto numTargets = Lua::GetObjectLength(l,2);
		std::vector<VectorR3> targets;
		targets.reserve(numTargets);
		for(auto i=decltype(numTargets){0};i<numTargets;++i)
		{
			Lua::PushInt(l,i +1);
			Lua::GetTableValue(l,2);
			auto *v = Lua::CheckVector(l,-1);
			targets.push_back(VectorR3(v->x,v->y,v->z));
			Lua::Pop(l,1);
		}
		jacobian.UpdatedSClampValue(targets.data());
	}));
	//classIkJacobian.def("SetCurrentMode",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.SetCurrentMode();
	//}));
	//classIkJacobian.def("GetCurrentMode",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.GetCurrentMode();
	//}));
	//classIkJacobian.def("SetDampingDLS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.SetDampingDLS();
	//}));
	classIkJacobian.def("Reset",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.Reset();
	}));
	/*classIkJacobian.def("CompareErrors",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CompareErrors();
	}));
	classIkJacobian.def("CountErrors",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CountErrors();
	}));*/
	classIkJacobian.def("GetRowCount",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		Lua::PushInt(l,jacobian.GetNumRows());
	}));
	classIkJacobian.def("GetColumnCount",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		Lua::PushInt(l,jacobian.GetNumCols());
	}));
	physMod[classIkJacobian];

	classTreeIkTree.scope[classTreeIkTreeNode];
	physMod[classTreeIkTree];

	auto classIkController = luabind::class_<IKController>("IKController");
	classIkController.def("GetEffectorName",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushString(l,ikController.GetEffectorName());
	}));
	classIkController.def("GetChainLength",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushInt(l,ikController.GetChainLength());
	}));
	classIkController.def("GetType",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushString(l,ikController.GetType());
	}));
	classIkController.def("SetEffectorName",static_cast<void(*)(lua_State*,IKController&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &effectorName) {
		ikController.SetEffectorName(effectorName);
	}));
	classIkController.def("SetChainLength",static_cast<void(*)(lua_State*,IKController&,uint32_t)>([](lua_State *l,IKController &ikController,uint32_t chainLength) {
		ikController.SetChainLength(chainLength);
	}));
	classIkController.def("SetType",static_cast<void(*)(lua_State*,IKController&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &type) {
		ikController.SetType(type);
	}));
	classIkController.def("SetMethod",static_cast<void(*)(lua_State*,IKController&,uint32_t)>([](lua_State *l,IKController &ikController,uint32_t method) {
		ikController.SetMethod(static_cast<util::ik::Method>(method));
	}));
	classIkController.def("GetMethod",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		Lua::PushInt(l,ikController.GetMethod());
	}));
	classIkController.def("GetKeyValues",static_cast<void(*)(lua_State*,IKController&)>([](lua_State *l,IKController &ikController) {
		auto &ikKeyValues = ikController.GetKeyValues();
		auto t = Lua::CreateTable(l);
		for(auto &pair : ikKeyValues)
		{
			Lua::PushString(l,pair.first);
			Lua::PushString(l,pair.second);
			Lua::SetTableValue(l,t);
		}
	}));
	classIkController.def("SetKeyValues",static_cast<void(*)(lua_State*,IKController&,luabind::object)>([](lua_State *l,IKController &ikController,luabind::object o) {
		Lua::CheckTable(l,2);
		auto &ikKeyValues = ikController.GetKeyValues();
		ikKeyValues.clear();
		ikKeyValues.reserve(Lua::GetObjectLength(l,2));

		Lua::PushNil(l);
		while(Lua::GetNextPair(l,2) != 0)
		{
			auto *key = Lua::CheckString(l,-2);
			auto *val = Lua::CheckString(l,-1);
			ikKeyValues[key] = val;
			Lua::Pop(l,1);
		}
	}));
	classIkController.def("SetKeyValue",static_cast<void(*)(lua_State*,IKController&,const std::string&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &key,const std::string &value) {
		auto &ikKeyValues = ikController.GetKeyValues();
		ikKeyValues[key] = value;
	}));
	classIkController.def("GetKeyValue",static_cast<void(*)(lua_State*,IKController&,const std::string&)>([](lua_State *l,IKController &ikController,const std::string &key) {
		auto &ikKeyValues = ikController.GetKeyValues();
		auto it = ikKeyValues.find(key);
		if(it == ikKeyValues.end())
			return;
		Lua::PushString(l,it->second);
	}));
	classIkController.add_static_constant("METHOD_SELECTIVELY_DAMPED_LEAST_SQUARE",umath::to_integral(util::ik::Method::SelectivelyDampedLeastSquare));
	classIkController.add_static_constant("METHOD_DAMPED_LEAST_SQUARES",umath::to_integral(util::ik::Method::DampedLeastSquares));
	classIkController.add_static_constant("METHOD_DAMPED_LEAST_SQUARES_WITH_SINGULAR_VALUE_DECOMPOSITION",umath::to_integral(util::ik::Method::DampedLeastSquaresWithSingularValueDecomposition));
	classIkController.add_static_constant("METHOD_PSEUDOINVERSE",umath::to_integral(util::ik::Method::Pseudoinverse));
	classIkController.add_static_constant("METHOD_JACOBIAN_TRANSPOSE",umath::to_integral(util::ik::Method::JacobianTranspose));
	classIkController.add_static_constant("METHOD_DEFAULT",umath::to_integral(util::ik::Method::Default));
	physMod[classIkController];

	auto classDef = luabind::class_<::PhysSoftBodyInfo>("SoftBodyInfo");
	Lua::PhysSoftBodyInfo::register_class(l,classDef);
	physMod[classDef];

	Lua::PhysConstraint::register_class(l,physMod);
	Lua::PhysCollisionObj::register_class(l,physMod);
	Lua::PhysObj::register_class(l,physMod);
	Lua::PhysContact::register_class(l,physMod);
	Lua::PhysShape::register_class(l,physMod);
	Lua::PhysKinematicCharacterController::register_class(l,physMod);
}
int Lua::physenv::raycast(lua_State *l)
{
	auto *data = Lua::CheckTraceData(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<TraceResult> res;
	auto r = game->RayCast(*data,&res);
	if(res.empty() || (r == false && (data->HasFlag(RayCastFlags::ReportAllResults) == false || res.size() == 1)))
	{
		Lua::PushBool(l,r);
		return 1;
	}
	auto table = Lua::CreateTable(l);
	if(data->HasFlag(RayCastFlags::ReportAllResults))
	{
		for(size_t i=0;i<res.size();i++)
		{
			auto &r = res[i];
			Lua_TraceData_FillTraceResultTable(l,r);
			lua_rawseti(l,table,i +1);
		}
	}
	else
		Lua_TraceData_FillTraceResultTable(l,res.back());
	return 1;
}
int Lua::physenv::sweep(lua_State *l)
{
	auto *data = Lua::CheckTraceData(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<TraceResult> res;
	auto r = game->Sweep(*data,&res);
	if(res.empty() || (r == false && (data->HasFlag(RayCastFlags::ReportAllResults) == false || res.size() == 1)))
	{
		Lua::PushBool(l,r);
		return 1;
	}
	auto table = Lua::CreateTable(l);
	if(data->HasFlag(RayCastFlags::ReportAllResults))
	{
		for(size_t i=0;i<res.size();i++)
		{
			auto &r = res[i];
			Lua_TraceData_FillTraceResultTable(l,r);
			lua_rawseti(l,table,i +1);
		}
	}
	else
		Lua_TraceData_FillTraceResultTable(l,res.back());
	return 1;
}
int Lua::physenv::overlap(lua_State *l)
{
	auto *data = Lua::CheckTraceData(l,1);
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<TraceResult> res;
	auto r = game->Overlap(*data,&res);
	if(res.empty() || (r == false && (data->HasFlag(RayCastFlags::ReportAllResults) == false || res.size() == 1)))
	{
		Lua::PushBool(l,r);
		return 1;
	}
	auto table = Lua::CreateTable(l);
	if(data->HasFlag(RayCastFlags::ReportAllResults))
	{
		for(size_t i=0;i<res.size();i++)
		{
			auto &r = res[i];
			Lua_TraceData_FillTraceResultTable(l,r);
			lua_rawseti(l,table,i +1);
		}
	}
	else
		Lua_TraceData_FillTraceResultTable(l,res.back());
	return 1;
}
int Lua::physenv::create_convex_hull_shape(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto &mat = Lua::CheckHandle<pragma::physics::IMaterial>(l,1);
	auto shape = env->CreateConvexHullShape(mat);
	if(shape == nullptr)
		return 0;
	shape->Push(l);
	return 1;
}

int Lua::physenv::create_box_shape(lua_State *l)
{
	auto *halfExtents = Lua::CheckVector(l,1);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto &mat = Lua::CheckHandle<pragma::physics::IMaterial>(l,2);
	auto shape = env->CreateBoxShape(*halfExtents,mat);
	if(shape == nullptr)
		return 0;
	shape->Push(l);
	return 1;
}

int Lua::physenv::create_capsule_shape(lua_State *l)
{
	auto halfWidth = Lua::CheckNumber(l,1);
	auto halfHeight = Lua::CheckNumber(l,2);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto &mat = Lua::CheckHandle<pragma::physics::IMaterial>(l,3);

	auto shape = env->CreateCapsuleShape(CFloat(halfWidth),CFloat(halfHeight),mat);
	if(shape == nullptr)
		return 0;
	shape->Push(l);
	return 1;
}

int Lua::physenv::create_sphere_shape(lua_State *l)
{
	auto radius = Lua::CheckNumber(l,1);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto &mat = Lua::CheckHandle<pragma::physics::IMaterial>(l,2);

	auto shape = env->CreateSphereShape(CFloat(radius),mat);
	if(shape == nullptr)
		return 0;
	shape->Push(l);
	return 1;
}

int Lua::physenv::create_cylinder_shape(lua_State *l)
{
	auto radius = Lua::CheckNumber(l,1);
	auto height = Lua::CheckNumber(l,2);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto &mat = Lua::CheckHandle<pragma::physics::IMaterial>(l,3);

	auto shape = env->CreateCylinderShape(radius,height,mat);
	if(shape == nullptr)
		return 0;
	shape->Push(l);
	return 1;
}

int Lua::physenv::create_compound_shape(lua_State *l)
{
	auto tShapes = 1;
	Lua::CheckTable(l,tShapes);
	std::vector<pragma::physics::IShape*> shapes {};
	auto numShapes = Lua::GetObjectLength(l,tShapes);
	shapes.reserve(numShapes);
	for(auto i=decltype(numShapes){0u};i<numShapes;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,tShapes);
		auto &shape = Lua::Check<pragma::physics::IShape>(l,2);
		shapes.push_back(&shape);
		Lua::Pop(l,1);
	}

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto shape = env->CreateCompoundShape(shapes);
	if(shape == nullptr)
		return 0;
	shape->Push(l);
	return 1;
}

int Lua::physenv::create_heightfield_terrain_shape(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;

	auto width = Lua::CheckInt(l,1);
	auto length = Lua::CheckInt(l,2);
	auto maxHeight = Lua::CheckNumber(l,3);
	auto upAxis = Lua::CheckInt(l,4);
	auto &mat = Lua::CheckHandle<pragma::physics::IMaterial>(l,5);

	auto shape = env->CreateHeightfieldTerrainShape(width,length,maxHeight,upAxis,mat);
	if(shape == nullptr)
		return 0;
	shape->Push(l);
	return 1;
}

int Lua::physenv::create_rigid_body(lua_State *l)
{
	auto &shape = Lua::Check<pragma::physics::IShape>(l,1);
	auto dynamic = true;
	if(Lua::IsSet(l,2))
		dynamic = Lua::CheckBool(l,2);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto body = env->CreateRigidBody(shape,dynamic);
	if(body == nullptr)
		return 0;
	body->Push(l);
	return 1;
}

int Lua::physenv::create_ghost_object(lua_State *l)
{
	auto &shape = Lua::Check<pragma::physics::IShape>(l,2);

	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto ghost = env->CreateGhostObject(shape);
	if(ghost == nullptr)
		return 0;
	ghost->Push(l);
	return 1;
}

int Lua::physenv::create_plane(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto &n = Lua::Check<Vector3>(l,1);
	auto d = Lua::CheckNumber(l,2);
	auto &mat = Lua::CheckHandle<pragma::physics::IMaterial>(l,3);
	auto plane = env->CreatePlane(n,d,mat);
	if(plane == nullptr)
		return 0;
	plane->Push(l);
	return 1;
}

int Lua::physenv::create_fixed_constraint(lua_State *l)
{
	auto &bodyA = Lua::CheckHandle<pragma::physics::IRigidBody>(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto &bodyB = Lua::CheckHandle<pragma::physics::IRigidBody>(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto constraint = env->CreateFixedConstraint(
		bodyA,*pivotA,*rotA,
		bodyB,*pivotB,*rotB
	);
	if(constraint == nullptr)
		return 0;
	constraint->Push(l);
	return 1;
}

int Lua::physenv::create_ball_socket_constraint(lua_State *l)
{
	auto &bodyA = Lua::CheckHandle<pragma::physics::IRigidBody>(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto &bodyB = Lua::CheckHandle<pragma::physics::IRigidBody>(l,3);
	auto *pivotB = Lua::CheckVector(l,4);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto constraint = env->CreateBallSocketConstraint(
		bodyA,*pivotA,
		bodyB,*pivotB
	);
	if(constraint == nullptr)
		return 0;
	constraint->Push(l);
	return 1;
}

int Lua::physenv::create_hinge_constraint(lua_State *l)
{
	auto &bodyA = Lua::CheckHandle<pragma::physics::IRigidBody>(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto &bodyB = Lua::CheckHandle<pragma::physics::IRigidBody>(l,3);
	auto *pivotB = Lua::CheckVector(l,4);
	auto *axis = Lua::CheckVector(l,5);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto constraint = env->CreateHingeConstraint(
		bodyA,*pivotA,
		bodyB,*pivotB,*axis
	);
	if(constraint == nullptr)
		return 0;
	constraint->Push(l);
	return 1;
}

int Lua::physenv::create_slider_constraint(lua_State *l)
{
	auto &bodyA = Lua::CheckHandle<pragma::physics::IRigidBody>(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto &bodyB = Lua::CheckHandle<pragma::physics::IRigidBody>(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto constraint = env->CreateSliderConstraint(
		bodyA,*pivotA,*rotA,
		bodyB,*pivotB,*rotB
	);
	if(constraint == nullptr)
		return 0;
	constraint->Push(l);
	return 1;
}

int Lua::physenv::create_cone_twist_constraint(lua_State *l)
{
	auto &bodyA = Lua::CheckHandle<pragma::physics::IRigidBody>(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto &bodyB = Lua::CheckHandle<pragma::physics::IRigidBody>(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto constraint = env->CreateConeTwistConstraint(
		bodyA,*pivotA,*rotA,
		bodyB,*pivotB,*rotB
	);
	if(constraint == nullptr)
		return 0;
	constraint->Push(l);
	return 1;
}

int Lua::physenv::create_DoF_constraint(lua_State *l)
{
	auto &bodyA = Lua::CheckHandle<pragma::physics::IRigidBody>(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto &bodyB = Lua::CheckHandle<pragma::physics::IRigidBody>(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto constraint = env->CreateDoFConstraint(
		bodyA,*pivotA,*rotA,
		bodyB,*pivotB,*rotB
	);
	if(constraint == nullptr)
		return 0;
	constraint->Push(l);
	return 1;
}

int Lua::physenv::create_dof_spring_constraint(lua_State *l)
{
	auto &bodyA = Lua::CheckHandle<pragma::physics::IRigidBody>(l,1);
	auto *pivotA = Lua::CheckVector(l,2);
	auto *rotA = Lua::CheckQuaternion(l,3);
	auto &bodyB = Lua::CheckHandle<pragma::physics::IRigidBody>(l,4);
	auto *pivotB = Lua::CheckVector(l,5);
	auto *rotB = Lua::CheckQuaternion(l,6);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto constraint = env->CreateDoFSpringConstraint(
		bodyA,*pivotA,*rotA,
		bodyB,*pivotB,*rotB
	);
	if(constraint == nullptr)
		return 0;
	constraint->Push(l);
	return 1;
}

int Lua::physenv::create_surface_material(lua_State *l)
{
	auto name = Lua::CheckString(l,1);
	auto friction = Lua::CheckNumber(l,2);
	auto restitution = Lua::CheckNumber(l,3);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &mat = game->CreateSurfaceMaterial(name,CFloat(friction),CFloat(restitution));
	Lua::Push<SurfaceMaterial*>(l,&mat);
	return 1;
}

int Lua::physenv::create_box_controller(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto argIdx = 1;
	auto &halfExtents = Lua::Check<Vector3>(l,argIdx++);
	auto stepHeight = Lua::CheckNumber(l,argIdx++);
	auto slopeLimit = 45.f;
	umath::Transform startTransform = {};
	if(Lua::IsSet(l,argIdx))
		slopeLimit = Lua::CheckNumber(l,argIdx++);
	if(Lua::IsSet(l,argIdx))
		startTransform = Lua::Check<umath::Transform>(l,argIdx++);
	auto controller = env->CreateBoxController(halfExtents,stepHeight,slopeLimit,startTransform);
	if(controller == nullptr)
		return 0;
	controller->Push(l);
	return 1;
}
int Lua::physenv::create_capsule_controller(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto argIdx = 1;
	auto halfWidth = Lua::CheckNumber(l,argIdx++);
	auto halfHeight = Lua::CheckNumber(l,argIdx++);
	auto stepHeight = Lua::CheckNumber(l,argIdx++);
	auto slopeLimit = 45.f;
	umath::Transform startTransform = {};
	if(Lua::IsSet(l,argIdx))
		slopeLimit = Lua::CheckNumber(l,argIdx++);
	if(Lua::IsSet(l,argIdx))
		startTransform = Lua::Check<umath::Transform>(l,argIdx++);
	auto controller = env->CreateCapsuleController(halfWidth,halfHeight,stepHeight,slopeLimit,startTransform);
	if(controller == nullptr)
		return 0;
	controller->Push(l);
	return 1;
}

int Lua::physenv::create_material(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *env = game->GetPhysicsEnvironment();
	if(env == nullptr)
		return 0;
	auto staticFriction = Lua::CheckNumber(l,1);
	auto dynamicFriction = Lua::CheckNumber(l,2);
	auto restitution = Lua::CheckNumber(l,3);
	auto mat = env->CreateMaterial(staticFriction,dynamicFriction,restitution);
	if(mat == nullptr)
		return 0;
	mat->Push(l);
	return 1;
}
int Lua::physenv::get_surface_material(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	SurfaceMaterial *mat = nullptr;
	if(Lua::IsNumber(l,1))
	{
		auto id = Lua::CheckInt(l,1);
		mat = game->GetSurfaceMaterial(CUInt32(id));
	}
	else
	{
		auto name = Lua::CheckString(l,1);
		mat = game->GetSurfaceMaterial(name);
	}
	if(mat == nullptr)
		return 0;
	Lua::Push<SurfaceMaterial*>(l,mat);
	return 1;
}
int Lua::physenv::get_surface_materials(lua_State *l)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &mats = game->GetSurfaceMaterials();
	Lua::CreateTable(l);
	auto idxTable = Lua::GetStackTop(l);
	for(UInt i=0;i<mats.size();i++)
	{
		auto &mat = mats[i];
		Lua::PushInt(l,i +1);
		Lua::Push<SurfaceMaterial*>(l,&mat);
		Lua::SetTableValue(l,idxTable);
	}
	return 1;
}

int Lua::physenv::create_character_controller(lua_State*)
{
	/*
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	btTransform startTransform;
	startTransform.setIdentity();
	startTransform.setOrigin(btVector3(0.f,0.f,0.f));
	btPairCachingGhostObject *ghost = new btPairCachingGhostObject;
	ghost->setWorldTransform(startTransform);

	btCapsuleShape *shape = new btCapsuleShape(width,height);
	ghost->setCollisionShape(shape);
	ghost->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);

	float stepHeight = 24.f;
	PhysKinematicCharacterController *controller = new PhysKinematicCharacterController(ghost,shape,stepHeight);
	controller->setGravity(0.f);
	*/
	/*PhysEnv *physEnv = game->GetPhysicsEnvironment();
	physEnv->addCollisionObject(ghost,btBroadphaseProxy::CharacterFilter,btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	physEnv->addAction(controller);

	Lua::Push<PhysKinematicCharacterController*>(l,controller);*/
	return 0;
}

int Lua::physenv::calc_torque_from_angular_velocity(lua_State *l)
{
	auto &angVel = *Lua::CheckVector(l,1);
	auto &invInertiaTensor = *Lua::CheckMat3(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto torque = angVel /static_cast<float>(dt);
	torque = glm::inverse(invInertiaTensor) *torque;
	Lua::Push<Vector3>(l,torque);
	return 1;
}
int Lua::physenv::calc_angular_velocity_from_torque(lua_State *l)
{
	auto &torque = *Lua::CheckVector(l,1);
	auto &invInertiaTensor = *Lua::CheckMat3(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto angVel = torque *invInertiaTensor *static_cast<float>(dt);
	Lua::Push<Vector3>(l,angVel);
	return 1;
}
int Lua::physenv::calc_force_from_linear_velocity(lua_State *l)
{
	auto &linVel = *Lua::CheckVector(l,1);
	auto mass = Lua::CheckNumber(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto force = (static_cast<float>(mass) *linVel) /static_cast<float>(dt);
	Lua::Push<Vector3>(l,force);
	return 1;
}
int Lua::physenv::calc_linear_velocity_from_force(lua_State *l)
{
	auto &force = *Lua::CheckVector(l,1);
	auto mass = Lua::CheckNumber(l,2);
	auto dt = Lua::CheckNumber(l,3);
	auto linVel = (force *static_cast<float>(dt)) /static_cast<float>(mass);
	Lua::Push<Vector3>(l,linVel);
	return 1;
}
