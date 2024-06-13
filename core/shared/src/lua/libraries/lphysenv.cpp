/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
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
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/converters/vector_converter_t.hpp"
#include "pragma/lua/converters/game_type_converters_t.hpp"
#include "pragma/physics/raytraces.h"
#include "pragma/model/modelmesh.h"
#include "pragma/lua/classes/lphyssoftbodyinfo.hpp"
#include "pragma/lua/policies/shared_from_this_policy.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
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
#include <luabind/copy_policy.hpp>
#include <sharedutils/magic_enum.hpp>

#include <luabind/detail/type_traits.hpp>

extern DLLNETWORK Engine *engine;

// #define ENABLE_DEPRECATED_PHYSICS

namespace Lua {
	namespace physenv {
		static Lua::var<bool, luabind::tableT<TraceResult>, TraceResult> raycast(lua_State *l, Game &game, const ::TraceData &traceData);
		static Lua::var<bool, luabind::tableT<TraceResult>, TraceResult> sweep(lua_State *l, Game &game, const ::TraceData &traceData);
		static Lua::var<bool, luabind::tableT<TraceResult>, TraceResult> overlap(lua_State *l, Game &game, const ::TraceData &traceData);
		static util::TSharedHandle<pragma::physics::IRigidBody> create_rigid_body(pragma::physics::IEnvironment *env, pragma::physics::IShape &shape, bool dynamic = true);
		static std::shared_ptr<pragma::physics::IConvexHullShape> create_convex_hull_shape(pragma::physics::IEnvironment *env, pragma::physics::IMaterial &material);

		static std::shared_ptr<pragma::physics::IConvexShape> create_convex_shape(pragma::physics::IEnvironment *env, const std::vector<Vector3> &verts, pragma::physics::IMaterial &material);
		static std::shared_ptr<pragma::physics::IConvexShape> create_convex_shape(pragma::physics::IEnvironment *env, const std::vector<Vector3> &verts, const std::vector<uint16_t> &tris, pragma::physics::IMaterial &material);
		static std::shared_ptr<pragma::physics::ITriangleShape> create_triangle_shape(pragma::physics::IEnvironment *env, const std::vector<Vector3> &verts, const std::vector<uint16_t> &tris, pragma::physics::IMaterial &material);
		static std::shared_ptr<pragma::physics::IConvexShape> create_box_shape(pragma::physics::IEnvironment *env, const Vector3 &halfExtents, pragma::physics::IMaterial &material);
		static std::shared_ptr<pragma::physics::IConvexShape> create_capsule_shape(pragma::physics::IEnvironment *env, float halfWidth, float halfHeight, pragma::physics::IMaterial &material);
		static std::shared_ptr<pragma::physics::IConvexShape> create_sphere_shape(pragma::physics::IEnvironment *env, float radius, pragma::physics::IMaterial &material);
		static std::shared_ptr<pragma::physics::IConvexShape> create_cylinder_shape(pragma::physics::IEnvironment *env, float radius, float height, pragma::physics::IMaterial &material);
		static std::shared_ptr<pragma::physics::ICompoundShape> create_compound_shape(pragma::physics::IEnvironment *env, std::vector<pragma::physics::IShape *> &shapes);
		static std::shared_ptr<pragma::physics::IShape> create_heightfield_terrain_shape(pragma::physics::IEnvironment *env, float width, float length, float maxHeight, float upAxis, pragma::physics::IMaterial &material);
		static util::TSharedHandle<pragma::physics::IGhostObject> create_ghost_object(pragma::physics::IEnvironment *env, pragma::physics::IShape &shape);
		static util::TSharedHandle<pragma::physics::ICollisionObject> create_plane(pragma::physics::IEnvironment *env, const Vector3 &n, double d, pragma::physics::IMaterial &material);
		static util::TSharedHandle<pragma::physics::IFixedConstraint> create_fixed_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Quat &rotB);
		static util::TSharedHandle<pragma::physics::IBallSocketConstraint> create_ball_socket_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB);
		static util::TSharedHandle<pragma::physics::IHingeConstraint> create_hinge_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Vector3 &axis);
		static util::TSharedHandle<pragma::physics::ISliderConstraint> create_slider_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Quat &rotB);
		static util::TSharedHandle<pragma::physics::IConeTwistConstraint> create_cone_twist_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB,
		  const Quat &rotB);
		static util::TSharedHandle<pragma::physics::IDoFConstraint> create_DoF_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Quat &rotB);
		static util::TSharedHandle<pragma::physics::IDoFSpringConstraint> create_dof_spring_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB,
		  const Quat &rotB);
		static SurfaceMaterial *create_surface_material(Game &game, const std::string &name, float friction, float restitution);

		static util::TSharedHandle<pragma::physics::IController> create_box_controller(pragma::physics::IEnvironment *env, const Vector3 &halfExtents, float stepHeight, float slopeLimit = 45.f, const umath::Transform &startTransform = {});
		static util::TSharedHandle<pragma::physics::IController> create_capsule_controller(pragma::physics::IEnvironment *env, float halfWidth, float halfHeight, float stepHeight, float slopeLimit = 45.f, const umath::Transform &startTransform = {});

		static std::shared_ptr<pragma::physics::IMaterial> create_material(pragma::physics::IEnvironment *env, float staticFriction, float dynamicFriction, float restitution);
		static luabind::tableT<SurfaceMaterial> get_surface_materials(lua_State *l, Game &game);

		static void create_character_controller(lua_State *);

		static Vector3 calc_torque_from_angular_velocity(const Vector3 &angVel, const Mat3 &invInertiaTensor, float dt);
		static Vector3 calc_angular_velocity_from_torque(const Vector3 &torque, const Mat3 &invInertiaTensor, float dt);
		static Vector3 calc_force_from_linear_velocity(const Vector3 &linVel, float mass, float dt);
		static Vector3 calc_linear_velocity_from_force(const Vector3 &force, float mass, float dt);
	};
};

static pragma::physics::VehicleCreateInfo create_standard_four_wheel_drive(lua_State *l, const std::array<Vector3, pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> &wheelCenterOffsets, float handBrakeTorque = 6'400'000.0, float maxSteeringAngle = 60.0)
{
	return pragma::physics::VehicleCreateInfo::CreateStandardFourWheelDrive(wheelCenterOffsets, handBrakeTorque, maxSteeringAngle);
}

extern std::ostream &operator<<(std::ostream &out, const umath::Transform &t);
extern std::ostream &operator<<(std::ostream &out, const umath::ScaledTransform &t);

void Lua::physenv::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();

	const auto *libName = "phys";
	auto &modPhys = lua.RegisterLibrary(libName);
	modPhys[
	  //luabind::def("create_character_controller",create_character_controller},
	  luabind::def("create_convex_shape", static_cast<std::shared_ptr<pragma::physics::IConvexShape> (*)(pragma::physics::IEnvironment *, const std::vector<Vector3> &, const std::vector<uint16_t> &, pragma::physics::IMaterial &)>(create_convex_shape)),
	  luabind::def("create_convex_shape", static_cast<std::shared_ptr<pragma::physics::IConvexShape> (*)(pragma::physics::IEnvironment *, const std::vector<Vector3> &, pragma::physics::IMaterial &)>(create_convex_shape)), luabind::def("create_triangle_shape", create_triangle_shape),
	  luabind::def("create_convex_hull_shape", create_convex_hull_shape), luabind::def("create_box_shape", create_box_shape), luabind::def("create_capsule_shape", create_capsule_shape), luabind::def("create_sphere_shape", create_sphere_shape),
	  luabind::def("create_cylinder_shape", create_cylinder_shape), luabind::def("create_compound_shape", create_compound_shape), luabind::def("create_heightfield_terrain_shape", create_heightfield_terrain_shape), luabind::def("create_rigid_body", create_rigid_body),
	  luabind::def("create_rigid_body", create_rigid_body, luabind::default_parameter_policy<3, true> {}), luabind::def("create_ghost_object", create_ghost_object), luabind::def("create_plane", create_plane), luabind::def("create_fixed_constraint", create_fixed_constraint),
	  luabind::def("create_ballsocket_constraint", create_ball_socket_constraint), luabind::def("create_hinge_constraint", create_hinge_constraint), luabind::def("create_slider_constraint", create_slider_constraint),
	  luabind::def("create_conetwist_constraint", create_cone_twist_constraint), luabind::def("create_dof_constraint", create_DoF_constraint), luabind::def("create_dof_spring_constraint", create_dof_spring_constraint), luabind::def("create_surface_material", create_surface_material),
	  luabind::def("create_material", create_material),
#ifdef _WIN32
	  luabind::def("create_box_controller", create_box_controller, luabind::meta::join<luabind::default_parameter_policy<4, 45.f>, luabind::default_parameter_policy<5, umath::Transform {}>>::type {}),
	  luabind::def("create_capsule_controller", create_capsule_controller, luabind::meta::join<luabind::default_parameter_policy<5, 45.f>, luabind::default_parameter_policy<6, umath::Transform {}>>::type {}),
#else
	  luabind::def("create_box_controller", create_box_controller),
	  luabind::def(
	    "create_box_controller", +[](pragma::physics::IEnvironment *env, const Vector3 &halfExtents, float stepHeight, float slopeLimit) { return create_box_controller(env, halfExtents, stepHeight, slopeLimit); }),
	  luabind::def(
	    "create_box_controller", +[](pragma::physics::IEnvironment *env, const Vector3 &halfExtents, float stepHeight) { return create_box_controller(env, halfExtents, stepHeight); }),
	  luabind::def("create_capsule_controller", create_capsule_controller),
	  luabind::def(
	    "create_capsule_controller", +[](pragma::physics::IEnvironment *env, float halfWidth, float halfHeight, float stepHeight, float slopeLimit) { return create_capsule_controller(env, halfWidth, halfHeight, stepHeight, slopeLimit); }),
	  luabind::def(
	    "create_capsule_controller", +[](pragma::physics::IEnvironment *env, float halfWidth, float halfHeight, float stepHeight) { return create_capsule_controller(env, halfWidth, halfHeight, stepHeight); }),
#endif
	  luabind::def("create_vehicle",
	    static_cast<util::TSharedHandle<pragma::physics::IVehicle> (*)(pragma::physics::IEnvironment *, const pragma::physics::VehicleCreateInfo &)>(
	      [](pragma::physics::IEnvironment *env, const pragma::physics::VehicleCreateInfo &vhcCreateInfo) -> util::TSharedHandle<pragma::physics::IVehicle> {
		      if(env == nullptr)
			      return nullptr;
		      return env->CreateVehicle(vhcCreateInfo);
	      })),
	  luabind::def("get_surface_material", static_cast<SurfaceMaterial *(Game::*)(const std::string &)>(&Game::GetSurfaceMaterial)), luabind::def("get_surface_material", static_cast<SurfaceMaterial *(Game::*)(uint32_t)>(&Game::GetSurfaceMaterial)),
	  luabind::def("get_surface_materials", get_surface_materials), luabind::def("get_generic_material", static_cast<pragma::physics::IMaterial *(*)(pragma::physics::IEnvironment *)>([](pragma::physics::IEnvironment *env) -> pragma::physics::IMaterial * {
		  if(env == nullptr)
			  return nullptr;
		  return &env->GetGenericMaterial();
	  })),
	  luabind::def("raycast", raycast), luabind::def("sweep", sweep), luabind::def("overlap", overlap),

	  luabind::def("calc_torque_from_angular_velocity", calc_torque_from_angular_velocity), luabind::def("calc_angular_velocity_from_torque", calc_angular_velocity_from_torque), luabind::def("calc_force_from_linear_velocity", calc_force_from_linear_velocity),
	  luabind::def("calc_linear_velocity_from_force", calc_linear_velocity_from_force)];

	Lua::RegisterLibraryEnums(l, libName,
	  {{"ACTIVATION_STATE_ACTIVE", umath::to_integral(pragma::physics::ICollisionObject::ActivationState::Active)}, {"ACTIVATION_STATE_ALWAYS_ACTIVE", umath::to_integral(pragma::physics::ICollisionObject::ActivationState::AlwaysActive)},
	    {"ACTIVATION_STATE_ALWAYS_INACTIVE", umath::to_integral(pragma::physics::ICollisionObject::ActivationState::AlwaysInactive)}, {"ACTIVATION_STATE_WAIT_FOR_DEACTIVATION", umath::to_integral(pragma::physics::ICollisionObject::ActivationState::WaitForDeactivation)},
	    {"ACTIVATION_STATE_COUNT", umath::to_integral(pragma::physics::ICollisionObject::ActivationState::Count)}});

	Lua::RegisterLibraryEnums(l, libName,
	  {{"TYPE_NONE", umath::to_integral(PHYSICSTYPE::NONE)}, {"TYPE_DYNAMIC", umath::to_integral(PHYSICSTYPE::DYNAMIC)}, {"TYPE_STATIC", umath::to_integral(PHYSICSTYPE::STATIC)}, {"TYPE_BOXCONTROLLER", umath::to_integral(PHYSICSTYPE::BOXCONTROLLER)},
	    {"TYPE_CAPSULECONTROLLER", umath::to_integral(PHYSICSTYPE::CAPSULECONTROLLER)}, {"TYPE_SOFTBODY", umath::to_integral(PHYSICSTYPE::SOFTBODY)},

	    {"COLLISIONMASK_NONE", umath::to_integral(CollisionMask::None)}, {"COLLISIONMASK_STATIC", umath::to_integral(CollisionMask::Static)}, {"COLLISIONMASK_DYNAMIC", umath::to_integral(CollisionMask::Dynamic)}, {"COLLISIONMASK_GENERIC", umath::to_integral(CollisionMask::Generic)},
	    {"COLLISIONMASK_PLAYER", umath::to_integral(CollisionMask::Player)}, {"COLLISIONMASK_NPC", umath::to_integral(CollisionMask::NPC)}, {"COLLISIONMASK_VEHICLE", umath::to_integral(CollisionMask::Vehicle)}, {"COLLISIONMASK_ITEM", umath::to_integral(CollisionMask::Item)},
	    {"COLLISIONMASK_ALL", umath::to_integral(CollisionMask::All)}, {"COLLISIONMASK_DEFAULT", umath::to_integral(CollisionMask::Default)}, {"COLLISIONMASK_PARTICLE", umath::to_integral(CollisionMask::Particle)}, {"COLLISIONMASK_CHARACTER", umath::to_integral(CollisionMask::Character)},
	    {"COLLISIONMASK_TRIGGER", umath::to_integral(CollisionMask::Trigger)}, {"COLLISIONMASK_WATER", umath::to_integral(CollisionMask::Water)}, {"COLLISIONMASK_WATER_SURFACE", umath::to_integral(CollisionMask::WaterSurface)},
	    {"COLLISIONMASK_PLAYER_HITBOX", umath::to_integral(CollisionMask::PlayerHitbox)}, {"COLLISIONMASK_NPC_HITBOX", umath::to_integral(CollisionMask::NPCHitbox)}, {"COLLISIONMASK_CHARACTER_HITBOX", umath::to_integral(CollisionMask::CharacterHitbox)},
	    {"COLLISIONMASK_ALL_HITBOX", umath::to_integral(CollisionMask::AllHitbox)}, {"COLLISIONMASK_NO_COLLISION", umath::to_integral(CollisionMask::NoCollision)},

	    {"RAYCAST_FLAG_BIT_REPORT_HIT_POSITION", umath::to_integral(RayCastFlags::ReportHitPosition)}, {"RAYCAST_FLAG_BIT_REPORT_HIT_NORMAL", umath::to_integral(RayCastFlags::ReportHitNormal)}, {"RAYCAST_FLAG_BIT_REPORT_HIT_UV", umath::to_integral(RayCastFlags::ReportHitUV)},
	    {"RAYCAST_FLAG_BIT_REPORT_ALL_RESULTS", umath::to_integral(RayCastFlags::ReportAllResults)}, {"RAYCAST_FLAG_BIT_REPORT_ANY_RESULT", umath::to_integral(RayCastFlags::ReportAnyResult)}, {"RAYCAST_FLAG_BIT_REPORT_BACK_FACE_HITS", umath::to_integral(RayCastFlags::ReportBackFaceHits)},
	    {"RAYCAST_FLAG_BIT_PRECISE", umath::to_integral(RayCastFlags::Precise)}, {"RAYCAST_FLAG_BIT_IGNORE_DYNAMIC", umath::to_integral(RayCastFlags::IgnoreDynamic)}, {"RAYCAST_FLAG_BIT_IGNORE_STATIC", umath::to_integral(RayCastFlags::IgnoreStatic)},
	    {"RAYCAST_FLAG_BIT_INVERT_FILTER", umath::to_integral(RayCastFlags::InvertFilter)}, {"RAYCAST_FLAG_DEFAULT", umath::to_integral(RayCastFlags::Default)}, {"RAYCAST_FLAG_NONE", umath::to_integral(RayCastFlags::None)}});

	auto &physMod = lua.RegisterLibrary(libName);
	auto classBase = luabind::class_<pragma::physics::IBase>("Base");
	physMod[classBase];

	auto classDefCon = luabind::class_<pragma::physics::IController, pragma::physics::IBase>("Controller");
	classDefCon.def("IsValid", static_cast<bool (*)(lua_State *, pragma::physics::IController *)>([](lua_State *l, pragma::physics::IController *hPhys) { return hPhys != nullptr; }));
	classDefCon.def("Remove", static_cast<void (*)(lua_State *, pragma::physics::IController &)>([](lua_State *l, pragma::physics::IController &hPhys) { hPhys.ClaimOwnership().Remove(); }));
	classDefCon.def("Spawn", &pragma::physics::IController::Spawn);
	classDefCon.def("Move", static_cast<void (*)(lua_State *, pragma::physics::IController &, const Vector3 &)>([](lua_State *l, pragma::physics::IController &hPhys, const Vector3 &disp) {
		auto v = disp;
		hPhys.Move(v);
	}));
	classDefCon.def("GetCollisionFlags", &pragma::physics::IController::GetCollisionFlags);
	classDefCon.def("GetGroundShape", &pragma::physics::IController::GetGroundShape);
	classDefCon.def("GetGroundBody", &pragma::physics::IController::GetGroundBody);
	classDefCon.def("GetGroundMaterial", &pragma::physics::IController::GetGroundMaterial);
	classDefCon.def("IsTouchingGround", &pragma::physics::IController::IsTouchingGround);
	classDefCon.def("GetGroundTouchPos", &pragma::physics::IController::GetGroundTouchPos);
	classDefCon.def("GetGroundTouchNormal", &pragma::physics::IController::GetGroundTouchNormal);
	classDefCon.def("GetDimensions", &pragma::physics::IController::GetDimensions);
	classDefCon.def("GetPos", &pragma::physics::IController::GetPos);
	classDefCon.def("GetFootPos", &pragma::physics::IController::GetFootPos);
	classDefCon.def("GetUpDirection", &pragma::physics::IController::GetUpDirection);
	classDefCon.def("GetMoveVelocity", &pragma::physics::IController::GetMoveVelocity, luabind::copy_policy<0> {});
	classDefCon.def("GetSlopeLimit", &pragma::physics::IController::GetSlopeLimit);
	classDefCon.def("GetStepHeight", &pragma::physics::IController::GetStepHeight);
	classDefCon.def("GetCollisionObject", static_cast<pragma::physics::ICollisionObject *(pragma::physics::IController::*)()>(&pragma::physics::IController::GetCollisionObject));
	classDefCon.def("GetShape", static_cast<pragma::physics::IConvexShape *(pragma::physics::IController::*)()>(&pragma::physics::IController::GetShape));
	classDefCon.def("SetDimensions", &pragma::physics::IController::SetDimensions);
	classDefCon.def("SetPos", &pragma::physics::IController::SetPos);
	classDefCon.def("SetFootPos", &pragma::physics::IController::SetFootPos);
	classDefCon.def("SetUpDirection", &pragma::physics::IController::SetUpDirection);
	classDefCon.def("SetMoveVelocity", &pragma::physics::IController::SetMoveVelocity);
	classDefCon.def("AddMoveVelocity", &pragma::physics::IController::AddMoveVelocity);
	classDefCon.def("Resize", &pragma::physics::IController::Resize);
	classDefCon.def("SetSlopeLimit", &pragma::physics::IController::SetSlopeLimit);
	classDefCon.def("SetStepHeight", &pragma::physics::IController::SetStepHeight);
	physMod[classDefCon];

	auto classDefVhc = luabind::class_<pragma::physics::IVehicle, pragma::physics::IBase>("Vehicle");
	classDefVhc.add_static_constant("GEAR_REVERSE", umath::to_integral(pragma::physics::IVehicle::Gear::Reverse));
	classDefVhc.add_static_constant("GEAR_NEUTRAL", umath::to_integral(pragma::physics::IVehicle::Gear::Neutral));
	classDefVhc.add_static_constant("GEAR_FIRST", umath::to_integral(pragma::physics::IVehicle::Gear::First));
	classDefVhc.add_static_constant("GEAR_SECOND", umath::to_integral(pragma::physics::IVehicle::Gear::Second));
	classDefVhc.add_static_constant("GEAR_THIRD", umath::to_integral(pragma::physics::IVehicle::Gear::Third));
	classDefVhc.add_static_constant("GEAR_FOURTH", umath::to_integral(pragma::physics::IVehicle::Gear::Fourth));
	classDefVhc.add_static_constant("GEAR_FIFTH", umath::to_integral(pragma::physics::IVehicle::Gear::Fifth));
	classDefVhc.add_static_constant("GEAR_SIXTH", umath::to_integral(pragma::physics::IVehicle::Gear::Sixth));
	classDefVhc.add_static_constant("GEAR_SEVENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Seventh));
	classDefVhc.add_static_constant("GEAR_EIGHTH", umath::to_integral(pragma::physics::IVehicle::Gear::Eighth));
	classDefVhc.add_static_constant("GEAR_NINTH", umath::to_integral(pragma::physics::IVehicle::Gear::Ninth));
	classDefVhc.add_static_constant("GEAR_TENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Tenth));
	classDefVhc.add_static_constant("GEAR_ELEVENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Eleventh));
	classDefVhc.add_static_constant("GEAR_TWELFTH", umath::to_integral(pragma::physics::IVehicle::Gear::Twelfth));
	classDefVhc.add_static_constant("GEAR_THIRTEENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Thirteenth));
	classDefVhc.add_static_constant("GEAR_FOURTEENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Fourteenth));
	classDefVhc.add_static_constant("GEAR_FIFTEENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Fifteenth));
	classDefVhc.add_static_constant("GEAR_SIXTEENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Sixteenth));
	classDefVhc.add_static_constant("GEAR_SEVENTEENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Seventeenth));
	classDefVhc.add_static_constant("GEAR_EIGHTEENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Eighteenth));
	classDefVhc.add_static_constant("GEAR_NINETEENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Nineteenth));
	classDefVhc.add_static_constant("GEAR_TWENTIETH", umath::to_integral(pragma::physics::IVehicle::Gear::Twentieth));
	classDefVhc.add_static_constant("GEAR_TWENTYFIRST", umath::to_integral(pragma::physics::IVehicle::Gear::Twentyfirst));
	classDefVhc.add_static_constant("GEAR_TWENTYSECOND", umath::to_integral(pragma::physics::IVehicle::Gear::Twentysecond));
	classDefVhc.add_static_constant("GEAR_TWENTYTHIRD", umath::to_integral(pragma::physics::IVehicle::Gear::Twentythird));
	classDefVhc.add_static_constant("GEAR_TWENTYFOURTH", umath::to_integral(pragma::physics::IVehicle::Gear::Twentyfourth));
	classDefVhc.add_static_constant("GEAR_TWENTYFIFTH", umath::to_integral(pragma::physics::IVehicle::Gear::Twentyfifth));
	classDefVhc.add_static_constant("GEAR_TWENTYSIXTH", umath::to_integral(pragma::physics::IVehicle::Gear::Twentysixth));
	classDefVhc.add_static_constant("GEAR_TWENTYSEVENTH", umath::to_integral(pragma::physics::IVehicle::Gear::Twentyseventh));
	classDefVhc.add_static_constant("GEAR_TWENTYEIGHTH", umath::to_integral(pragma::physics::IVehicle::Gear::Twentyeighth));
	classDefVhc.add_static_constant("GEAR_TWENTYNINTH", umath::to_integral(pragma::physics::IVehicle::Gear::Twentyninth));
	classDefVhc.add_static_constant("GEAR_THIRTIETH", umath::to_integral(pragma::physics::IVehicle::Gear::Thirtieth));
	classDefVhc.add_static_constant("GEAR_COUNT", umath::to_integral(pragma::physics::IVehicle::Gear::Count));
	classDefVhc.def("IsValid", static_cast<bool (*)(lua_State *, pragma::physics::IVehicle *)>([](lua_State *l, pragma::physics::IVehicle *hPhys) { return hPhys != nullptr; }));
	classDefVhc.def("Remove", static_cast<void (*)(lua_State *, pragma::physics::IVehicle *)>([](lua_State *l, pragma::physics::IVehicle *hPhys) {
		if(Lua::CheckHandle<pragma::physics::IVehicle>(l, hPhys) == false)
			return;
		hPhys->ClaimOwnership().Remove();
	}));
	classDefVhc.def("GetCollisionObject", static_cast<pragma::physics::ICollisionObject *(pragma::physics::IVehicle::*)()>(&pragma::physics::IVehicle::GetCollisionObject));
	classDefVhc.def("SetUseDigitalInputs", &pragma::physics::IVehicle::SetUseDigitalInputs);
	classDefVhc.def("SetBrakeFactor", &pragma::physics::IVehicle::SetBrakeFactor);
	classDefVhc.def("SetHandBrakeFactor", &pragma::physics::IVehicle::SetHandbrakeFactor);
	classDefVhc.def("SetAccelerationFactor", &pragma::physics::IVehicle::SetAccelerationFactor);
	classDefVhc.def("SetSteerFactor", &pragma::physics::IVehicle::SetSteerFactor);
	classDefVhc.def("SetGear", &pragma::physics::IVehicle::SetGear);
	classDefVhc.def("SetGearDown", &pragma::physics::IVehicle::SetGearDown);
	classDefVhc.def("SetGearUp", &pragma::physics::IVehicle::SetGearUp);
	classDefVhc.def("SetGearSwitchTime", &pragma::physics::IVehicle::SetGearSwitchTime);
	classDefVhc.def("ChangeToGear", &pragma::physics::IVehicle::ChangeToGear);
	classDefVhc.def("SetUseAutoGears", &pragma::physics::IVehicle::SetUseAutoGears);
	classDefVhc.def("ShouldUseAutoGears", &pragma::physics::IVehicle::ShouldUseAutoGears);
	classDefVhc.def("GetCurrentGear", &pragma::physics::IVehicle::GetCurrentGear);
	classDefVhc.def("GetEngineRotationSpeed", &pragma::physics::IVehicle::GetEngineRotationSpeed);
	classDefVhc.def("SetRestState", &pragma::physics::IVehicle::SetRestState);
	classDefVhc.def("ResetControls", &pragma::physics::IVehicle::ResetControls);
	classDefVhc.def("SetWheelRotationAngle", &pragma::physics::IVehicle::SetWheelRotationAngle);
	classDefVhc.def("SetWheelRotationSpeed", &pragma::physics::IVehicle::SetWheelRotationSpeed);
	classDefVhc.def("IsInAir", &pragma::physics::IVehicle::IsInAir);
	classDefVhc.def("GetWheelCount", &pragma::physics::IVehicle::GetWheelCount);
	classDefVhc.def("GetForwardSpeed", &pragma::physics::IVehicle::GetForwardSpeed);
	classDefVhc.def("GetSidewaysSpeed", &pragma::physics::IVehicle::GetSidewaysSpeed);
	physMod[classDefVhc];

	auto classMat = luabind::class_<pragma::physics::IMaterial, pragma::physics::IBase>("Material");
	classMat.def(
	  "__tostring", +[](const pragma::physics::IMaterial &mat) -> std::string {
		  std::stringstream ss;
		  ss << "PhysMaterial[SFriction:" << mat.GetStaticFriction() << "][DFriction:" << mat.GetDynamicFriction() << "][Restitution:" << mat.GetRestitution() << "]";
		  auto *surfMat = mat.GetSurfaceMaterial();
		  ss << "[SurfMat:" << (surfMat ? surfMat->GetIdentifier() : "NULL") << "]";
		  return ss.str();
	  });
	classMat.def("SetFriction", &pragma::physics::IMaterial::SetFriction);
	classMat.def("GetStaticFriction", &pragma::physics::IMaterial::GetStaticFriction);
	classMat.def("SetStaticFriction", &pragma::physics::IMaterial::SetStaticFriction);
	classMat.def("SetDynamicFriction", &pragma::physics::IMaterial::SetDynamicFriction);
	classMat.def("GetDynamicFriction", &pragma::physics::IMaterial::GetDynamicFriction);
	classMat.def("GetRestitution", &pragma::physics::IMaterial::GetRestitution);
	classMat.def("SetRestitution", &pragma::physics::IMaterial::SetRestitution);
	classMat.def("SetSurfaceMaterial", &pragma::physics::IMaterial::SetSurfaceMaterial);
	classMat.def("GetSurfaceMaterial", &pragma::physics::IMaterial::GetSurfaceMaterial);
	physMod[classMat];

	auto classDefRayCastData = luabind::class_<::TraceData>("RayCastData");
	classDefRayCastData.def(luabind::constructor<>());
	classDefRayCastData.def(
	  "__tostring", +[](const ::TraceData &data) -> std::string {
		  std::stringstream ss;
		  ss << "RayCastData[Flags:" << magic_enum::enum_name(data.GetFlags()) << "]";
		  auto &src = data.GetSource();
		  ss << "[Source:" << src << "]";
		  auto &dst = data.GetTarget();
		  ss << "[Target:" << dst << "]";
		  ss << "[FilterGroup:" << magic_enum::enum_name(data.GetCollisionFilterGroup()) << "]";
		  ss << "[FilterMask:" << magic_enum::enum_name(data.GetCollisionFilterMask()) << "]";
		  return ss.str();
	  });
	classDefRayCastData.def("SetShape", static_cast<void (*)(lua_State *, ::TraceData &, const pragma::physics::IConvexShape &)>(&Lua::TraceData::SetSource));
	classDefRayCastData.def("SetSource", static_cast<void (::TraceData::*)(const Vector3 &)>(&::TraceData::SetSource));
	classDefRayCastData.def("SetSourceRotation", &::TraceData::SetSourceRotation);
	classDefRayCastData.def("SetSource", static_cast<void (::TraceData::*)(const umath::Transform &)>(&::TraceData::SetSource));
	classDefRayCastData.def("SetTarget", static_cast<void (::TraceData::*)(const Vector3 &)>(&::TraceData::SetTarget));
	classDefRayCastData.def("SetTargetRotation", &::TraceData::SetTargetRotation);
	classDefRayCastData.def("SetTarget", static_cast<void (::TraceData::*)(const umath::Transform &)>(&::TraceData::SetTarget));
	classDefRayCastData.def("SetRotation", &::TraceData::SetRotation);
	classDefRayCastData.def("SetFlags", &Lua::TraceData::SetFlags);
	classDefRayCastData.def("SetFilter", &Lua::TraceData::SetFilter);
	classDefRayCastData.def("SetCollisionFilterMask", &Lua::TraceData::SetCollisionFilterMask);
	classDefRayCastData.def("SetCollisionFilterGroup", &Lua::TraceData::SetCollisionFilterGroup);
	classDefRayCastData.def("GetSourceTransform", &Lua::TraceData::GetSourceTransform);
	classDefRayCastData.def("GetTargetTransform", &Lua::TraceData::GetTargetTransform);
	classDefRayCastData.def("GetSourceOrigin", &Lua::TraceData::GetSourceOrigin);
	classDefRayCastData.def("GetTargetOrigin", &Lua::TraceData::GetTargetOrigin);
	classDefRayCastData.def("GetSourceRotation", &Lua::TraceData::GetSourceRotation);
	classDefRayCastData.def("GetTargetRotation", &Lua::TraceData::GetTargetRotation);
	classDefRayCastData.def("GetDistance", &Lua::TraceData::GetDistance);
	classDefRayCastData.def("GetDirection", &Lua::TraceData::GetDirection);
	physMod[classDefRayCastData];

	auto classDefRayCastResult = luabind::class_<TraceResult>("RayCastResult");
	classDefRayCastResult.def(
	  "__tostring", +[](const ::TraceResult &res) -> std::string {
		  std::stringstream ss;
		  ss << "RayCastResult";
		  ss << "[HitType:" << magic_enum::enum_name(res.hitType) << "]";
		  ss << "[HitPos:" << res.position << "]";
		  ss << "[HitNorm:" << res.normal << "]";

		  ss << "[HitEnt:";
		  if(res.entity.IsValid())
			  const_cast<BaseEntity *>(res.entity.get())->print(ss);
		  else
			  ss << "NULL";
		  ss << "]";

		  ss << "[HitMat:";
		  auto *mat = const_cast<::TraceResult &>(res).GetMaterial();
		  if(mat)
			  ss << mat->GetName();
		  else
			  ss << "NULL";
		  ss << "]";

		  ss << "[Frac:" << res.fraction << "]";
		  ss << "[StartPos:" << res.startPosition << "]";
		  return ss.str();
	  });
	classDefRayCastResult.add_static_constant("HIT_TYPE_BLOCK", umath::to_integral(RayCastHitType::Block));
	classDefRayCastResult.add_static_constant("HIT_TYPE_TOUCH", umath::to_integral(RayCastHitType::Touch));
	classDefRayCastResult.add_static_constant("HIT_TYPE_NONE", umath::to_integral(RayCastHitType::None));
	classDefRayCastResult.def_readonly("hitType", reinterpret_cast<std::underlying_type_t<decltype(TraceResult::hitType)> TraceResult::*>(&TraceResult::hitType));
	classDefRayCastResult.property("entity", static_cast<void (*)(lua_State *, TraceResult &)>([](lua_State *l, TraceResult &tr) {
		if(tr.entity.valid() == false)
			return;
		lua_pushentity(l, tr.entity);
	}));
	classDefRayCastResult.def_readonly("physObj", &TraceResult::physObj);
	classDefRayCastResult.def_readonly("fraction", &TraceResult::fraction);
	classDefRayCastResult.def_readonly("distance", &TraceResult::distance);
	classDefRayCastResult.def_readonly("normal", &TraceResult::normal);
	classDefRayCastResult.def_readonly("position", &TraceResult::position);
	classDefRayCastResult.def_readonly("startPosition", &TraceResult::startPosition);
	classDefRayCastResult.property("colObj", static_cast<pragma::physics::ICollisionObject *(*)(lua_State *, TraceResult &)>([](lua_State *l, TraceResult &tr) -> pragma::physics::ICollisionObject * { return tr.collisionObj.Get(); }));
	classDefRayCastResult.property("mesh", static_cast<std::shared_ptr<::ModelMesh> (*)(lua_State *, TraceResult &)>([](lua_State *l, TraceResult &tr) -> std::shared_ptr<::ModelMesh> {
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
		tr.GetMeshes(&mesh, &subMesh);
		if(mesh == nullptr)
			return nullptr;
		return mesh->shared_from_this();
	}));
	classDefRayCastResult.property("subMesh", static_cast<std::shared_ptr<::ModelSubMesh> (*)(lua_State *, TraceResult &)>([](lua_State *l, TraceResult &tr) -> std::shared_ptr<::ModelSubMesh> {
		ModelMesh *mesh = nullptr;
		ModelSubMesh *subMesh = nullptr;
		tr.GetMeshes(&mesh, &subMesh);
		if(subMesh == nullptr)
			return nullptr;
		return subMesh->shared_from_this();
	}));
	classDefRayCastResult.property("material", static_cast<Material *(TraceResult::*)()>(&TraceResult::GetMaterial));
	classDefRayCastResult.property("materialName", static_cast<std::optional<std::string> (*)(lua_State *, TraceResult &)>([](lua_State *l, TraceResult &tr) -> std::optional<std::string> {
		std::string mat;
		if(tr.GetMaterial(mat) == false)
			return {};
		return mat;
	}));
	physMod[classDefRayCastResult];

	auto classDefChassisCreateInfo = luabind::class_<pragma::physics::ChassisCreateInfo>("ChassisCreateInfo");
	classDefChassisCreateInfo.def(luabind::constructor<>());
	classDefChassisCreateInfo.def_readwrite("momentOfInertia", &pragma::physics::ChassisCreateInfo::momentOfInertia);
	classDefChassisCreateInfo.def_readwrite("centerOfMass", &pragma::physics::ChassisCreateInfo::centerOfMass);
	classDefChassisCreateInfo.def("AddShapeIndex", static_cast<void (*)(lua_State *, pragma::physics::VehicleCreateInfo &, uint32_t)>([](lua_State *l, pragma::physics::VehicleCreateInfo &vhcCreateInfo, uint32_t shapeIndex) { vhcCreateInfo.chassis.shapeIndices.push_back(shapeIndex); }));
	classDefChassisCreateInfo.def("GetShapeIndices", static_cast<std::vector<uint32_t> (*)(lua_State *, pragma::physics::VehicleCreateInfo &)>([](lua_State *l, pragma::physics::VehicleCreateInfo &vhcCreateInfo) -> std::vector<uint32_t> { return vhcCreateInfo.chassis.shapeIndices; }));
	physMod[classDefChassisCreateInfo];

	auto classDefSuspensionInfo = luabind::class_<pragma::physics::WheelCreateInfo::SuspensionInfo>("SuspensionInfo");
	classDefSuspensionInfo.def(luabind::constructor<>());
	classDefSuspensionInfo.def_readwrite("maxCompression", &pragma::physics::WheelCreateInfo::SuspensionInfo::maxCompression);
	classDefSuspensionInfo.def_readwrite("maxDroop", &pragma::physics::WheelCreateInfo::SuspensionInfo::maxDroop);
	classDefSuspensionInfo.def_readwrite("springStrength", &pragma::physics::WheelCreateInfo::SuspensionInfo::springStrength);
	classDefSuspensionInfo.def_readwrite("springDamperRate", &pragma::physics::WheelCreateInfo::SuspensionInfo::springDamperRate);
	classDefSuspensionInfo.def_readwrite("camberAngleAtRest", &pragma::physics::WheelCreateInfo::SuspensionInfo::camberAngleAtRest);
	classDefSuspensionInfo.def_readwrite("camberAngleAtMaxDroop", &pragma::physics::WheelCreateInfo::SuspensionInfo::camberAngleAtMaxDroop);
	classDefSuspensionInfo.def_readwrite("camberAngleAtMaxCompression", &pragma::physics::WheelCreateInfo::SuspensionInfo::camberAngleAtMaxCompression);

	auto classDefVhcCreateInfo = luabind::class_<pragma::physics::VehicleCreateInfo>("VehicleCreateInfo");
	classDefVhcCreateInfo.scope[luabind::def("CreateStandardFourWheelDrive",
	  static_cast<void (*)(lua_State *, const std::array<Vector3, pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> &, float, float)>(
	    [](lua_State *l, const std::array<Vector3, pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> &wheelCenterOffsets, float handBrakeTorque, float maxSteeringAngle) { create_standard_four_wheel_drive(l, wheelCenterOffsets, handBrakeTorque, maxSteeringAngle); }))];
	classDefVhcCreateInfo.scope[luabind::def("CreateStandardFourWheelDrive",
	  static_cast<void (*)(lua_State *, const std::array<Vector3, pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> &, float)>(
	    [](lua_State *l, const std::array<Vector3, pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> &wheelCenterOffsets, float handBrakeTorque) { create_standard_four_wheel_drive(l, wheelCenterOffsets, handBrakeTorque); }))];
	classDefVhcCreateInfo.scope[luabind::def("CreateStandardFourWheelDrive",
	  static_cast<void (*)(lua_State *, const std::array<Vector3, pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> &)>(
	    [](lua_State *l, const std::array<Vector3, pragma::physics::VehicleCreateInfo::WHEEL_COUNT_4W_DRIVE> &wheelCenterOffsets) { create_standard_four_wheel_drive(l, wheelCenterOffsets); }))];
	classDefVhcCreateInfo.add_static_constant("WHEEL_DRIVE_FRONT", umath::to_integral(pragma::physics::VehicleCreateInfo::WheelDrive::Front));
	classDefVhcCreateInfo.add_static_constant("WHEEL_DRIVE_REAR", umath::to_integral(pragma::physics::VehicleCreateInfo::WheelDrive::Rear));
	classDefVhcCreateInfo.add_static_constant("WHEEL_DRIVE_FOUR", umath::to_integral(pragma::physics::VehicleCreateInfo::WheelDrive::Four));
	classDefVhcCreateInfo.def(luabind::constructor<>());
	classDefVhcCreateInfo.def_readwrite("actor", &pragma::physics::VehicleCreateInfo::actor);
	classDefVhcCreateInfo.def_readwrite("chassis", &pragma::physics::VehicleCreateInfo::chassis);
	classDefVhcCreateInfo.def_readwrite("maxEngineTorque", &pragma::physics::VehicleCreateInfo::maxEngineTorque);
	classDefVhcCreateInfo.def_readwrite("maxEngineRotationSpeed", &pragma::physics::VehicleCreateInfo::maxEngineRotationSpeed);
	classDefVhcCreateInfo.def_readwrite("gearSwitchTime", &pragma::physics::VehicleCreateInfo::gearSwitchTime);
	classDefVhcCreateInfo.def_readwrite("clutchStrength", &pragma::physics::VehicleCreateInfo::clutchStrength);
	classDefVhcCreateInfo.def_readwrite("gravityFactor", &pragma::physics::VehicleCreateInfo::gravityFactor);
	classDefVhcCreateInfo.def_readwrite("wheelDrive", reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::VehicleCreateInfo::wheelDrive)> pragma::physics::VehicleCreateInfo::*>(&pragma::physics::VehicleCreateInfo::wheelDrive));
	classDefVhcCreateInfo.def("AddWheel", static_cast<void (*)(lua_State *, pragma::physics::VehicleCreateInfo &, pragma::physics::WheelCreateInfo &)>([](lua_State *l, pragma::physics::VehicleCreateInfo &vhcCreateInfo, pragma::physics::WheelCreateInfo &wheelCreateInfo) {
		auto wheelType = umath::to_integral(pragma::physics::VehicleCreateInfo::GetWheelType(wheelCreateInfo));
		auto it = std::find_if(vhcCreateInfo.wheels.begin(), vhcCreateInfo.wheels.end(), [wheelType](const pragma::physics::WheelCreateInfo &wheelDescOther) { return wheelType < umath::to_integral(pragma::physics::VehicleCreateInfo::GetWheelType(wheelDescOther)); });
		// Wheels have to be inserted in the correct order! (FrontLeft -> FrontRight -> RearLeft -> RearRight)
		vhcCreateInfo.wheels.insert(it, wheelCreateInfo);
	}));
	classDefVhcCreateInfo.def("GetWheels", static_cast<luabind::tableT<pragma::physics::WheelCreateInfo> (*)(lua_State *, pragma::physics::VehicleCreateInfo &)>([](lua_State *l, pragma::physics::VehicleCreateInfo &vhcCreateInfo) -> luabind::tableT<pragma::physics::WheelCreateInfo> {
		auto t = luabind::newtable(l);
		auto idx = 1;
		for(auto &wheel : vhcCreateInfo.wheels)
			t[idx++] = &wheel;
		return t;
	}));
	classDefVhcCreateInfo.def("AddAntiRollBar",
	  static_cast<void (*)(lua_State *, pragma::physics::VehicleCreateInfo &, pragma::physics::VehicleCreateInfo::AntiRollBar &)>(
	    [](lua_State *l, pragma::physics::VehicleCreateInfo &vhcCreateInfo, pragma::physics::VehicleCreateInfo::AntiRollBar &antiRollBar) { vhcCreateInfo.antiRollBars.push_back(antiRollBar); }));
	classDefVhcCreateInfo.def("GetAntiRollBars",
	  static_cast<luabind::tableT<pragma::physics::VehicleCreateInfo::AntiRollBar> (*)(lua_State *, pragma::physics::VehicleCreateInfo &)>([](lua_State *l, pragma::physics::VehicleCreateInfo &vhcCreateInfo) -> luabind::tableT<pragma::physics::VehicleCreateInfo::AntiRollBar> {
		  auto t = luabind::newtable(l);
		  auto idx = 1;
		  for(auto &antiRollBar : vhcCreateInfo.antiRollBars)
			  t[idx++] = &antiRollBar;
		  return t;
	  }));
	classDefVhcCreateInfo.scope[classDefSuspensionInfo];

	auto classDefAntiRollBar = luabind::class_<pragma::physics::VehicleCreateInfo::AntiRollBar>("AntiRollBar");
	classDefAntiRollBar.def(luabind::constructor<>());
	classDefAntiRollBar.def_readwrite("wheel0", reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::VehicleCreateInfo::AntiRollBar::wheel0)> pragma::physics::VehicleCreateInfo::AntiRollBar::*>(&pragma::physics::VehicleCreateInfo::AntiRollBar::wheel0));
	classDefAntiRollBar.def_readwrite("wheel1", reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::VehicleCreateInfo::AntiRollBar::wheel1)> pragma::physics::VehicleCreateInfo::AntiRollBar::*>(&pragma::physics::VehicleCreateInfo::AntiRollBar::wheel1));
	classDefAntiRollBar.def_readwrite("stiffness", &pragma::physics::VehicleCreateInfo::AntiRollBar::stiffness);
	classDefVhcCreateInfo.scope[classDefAntiRollBar];
	physMod[classDefVhcCreateInfo];

	auto classDefWheelCreateInfo = luabind::class_<pragma::physics::WheelCreateInfo>("WheelCreateInfo");
	classDefWheelCreateInfo.scope[luabind::def("CreateStandardFrontWheel", &pragma::physics::WheelCreateInfo::CreateStandardFrontWheel)];
	classDefWheelCreateInfo.scope[luabind::def("CreateStandardRearWheel", &pragma::physics::WheelCreateInfo::CreateStandardRearWheel)];
	classDefWheelCreateInfo.add_static_constant("FLAG_NONE", umath::to_integral(pragma::physics::WheelCreateInfo::Flags::None));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_FRONT", umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Front));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_REAR", umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Rear));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_LEFT", umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Left));
	classDefWheelCreateInfo.add_static_constant("FLAG_BIT_RIGHT", umath::to_integral(pragma::physics::WheelCreateInfo::Flags::Right));
	classDefWheelCreateInfo.def(luabind::constructor<>());
	classDefWheelCreateInfo.def_readwrite("suspension", &pragma::physics::WheelCreateInfo::suspension);
	classDefWheelCreateInfo.def_readwrite("width", &pragma::physics::WheelCreateInfo::width);
	classDefWheelCreateInfo.def_readwrite("radius", &pragma::physics::WheelCreateInfo::radius);
	classDefWheelCreateInfo.def_readwrite("shapeIndex", &pragma::physics::WheelCreateInfo::shapeIndex);
	classDefWheelCreateInfo.def_readwrite("tireType", &pragma::physics::WheelCreateInfo::tireType);
	classDefWheelCreateInfo.def_readwrite("flags", reinterpret_cast<std::underlying_type_t<decltype(pragma::physics::WheelCreateInfo::flags)> pragma::physics::WheelCreateInfo::*>(&pragma::physics::WheelCreateInfo::flags));
	classDefWheelCreateInfo.def_readwrite("momentOfInertia", &pragma::physics::WheelCreateInfo::momentOfInertia);
	classDefWheelCreateInfo.def_readwrite("chassisOffset", &pragma::physics::WheelCreateInfo::chassisOffset);
	classDefWheelCreateInfo.def_readwrite("maxHandBrakeTorque", &pragma::physics::WheelCreateInfo::maxHandbrakeTorque);
	classDefWheelCreateInfo.def_readwrite("maxSteeringAngle", &pragma::physics::WheelCreateInfo::maxSteeringAngle);
	physMod[classDefWheelCreateInfo];

	auto classTreeIkTree = luabind::class_<Tree>("IKTree");
	classTreeIkTree.scope[luabind::def("Create", static_cast<std::shared_ptr<Tree> (*)(lua_State *)>([](lua_State *l) { return std::make_shared<Tree>(); }))];
#ifdef ENABLE_DEPRECATED_PHYSICS
	classTreeIkTree.def("Draw", static_cast<void (*)(lua_State *, Tree &)>([](lua_State *l, Tree &tree) {
		auto *game = engine->GetNetworkState(l)->GetGameState();
		auto fGetLocalTransform = [](const Node *node, btTransform &act) {
			btVector3 axis = btVector3(node->v.x, node->v.y, node->v.z);
			btQuaternion rot(0, 0, 0, 1);
			if(axis.length()) {
				rot = btQuaternion(axis, node->theta);
			}
			act.setIdentity();
			act.setRotation(rot);
			act.setOrigin(btVector3(node->r.x, node->r.y, node->r.z));
		};
		std::function<void(Node *, const btTransform &)> fDrawTree = nullptr;
		fDrawTree = [&fGetLocalTransform, &fDrawTree, game](Node *node, const btTransform &tr) {
			btVector3 lineColor = btVector3(0, 0, 0);
			int lineWidth = 2;
			auto fUpdateLine = [game](int32_t tIdx, const Vector3 &start, const Vector3 &end, const Color &col) {
				/*auto it = m_dbgObjects.find(tIdx);
				if(it == m_dbgObjects.end())
					it = m_dbgObjects.insert(std::make_pair(tIdx,DebugRenderer::DrawLine(start,end,col))).first;
				auto &wo = static_cast<DebugRenderer::WorldObject&>(*m_dbgObjects.at(tIdx));
				wo.GetVertices().at(0) = start;
				wo.GetVertices().at(1) = end;
				wo.UpdateVertexBuffer();*/
				game->DrawLine(start, end, col, 0.05f);
				//DebugRenderer::DrawLine(start,end,col,0.05f);
			};
			if(node != 0) {
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
				fUpdateLine(1, enPos, enPos + enForward * static_cast<float>(0.05f / PhysEnv::WORLD_SCALE), Color::Red);
				fUpdateLine(2, enPos, enPos + enRight * static_cast<float>(0.05f / PhysEnv::WORLD_SCALE), Color::Lime);
				fUpdateLine(3, enPos, enPos + enUp * static_cast<float>(0.05f / PhysEnv::WORLD_SCALE), Color::Aqua);

				btVector3 axisLocal = btVector3(node->v.x, node->v.y, node->v.z);
				btVector3 axisWorld = tr.getBasis() * axisLocal;

				fUpdateLine(4, enPos, enPos + 0.1f * uvec::create(axisWorld), Color::Yellow);

				//node->DrawNode(node == root);	// Recursively draw node and update ModelView matrix
				if(node->left) {
					btTransform act;
					fGetLocalTransform(node->left, act);

					btTransform trl = tr * act;
					auto trOrigin = uvec::create(tr.getOrigin() / PhysEnv::WORLD_SCALE);
					auto trlOrigin = uvec::create(trl.getOrigin() / PhysEnv::WORLD_SCALE);
					fUpdateLine(5, trOrigin, trlOrigin, Color::Maroon);
					fDrawTree(node->left, trl); // Draw tree of children recursively
				}
				//	glPopMatrix();
				if(node->right) {
					btTransform act;
					fGetLocalTransform(node->right, act);
					btTransform trr = tr * act;
					auto trOrigin = uvec::create(tr.getOrigin() / PhysEnv::WORLD_SCALE);
					auto trrOrigin = uvec::create(trr.getOrigin() / PhysEnv::WORLD_SCALE);
					fUpdateLine(6, trOrigin, trrOrigin, Color::Silver);
					fDrawTree(node->right, trr); // Draw right siblings recursively
				}
			}
		};
		auto fRenderScene = [&fGetLocalTransform, &fDrawTree](Tree &tree) {
			btTransform act;
			fGetLocalTransform(tree.GetRoot(), act);
			fDrawTree(tree.GetRoot(), act);

			//btVector3 pos = btVector3(targetaa[0].x, targetaa[0].y, targetaa[0].z);
			//btQuaternion orn(0, 0, 0, 1);
		};
		fRenderScene(tree);
	}));
#endif
	classTreeIkTree.def("GetNodeCount", &Tree::GetNumNode);
	classTreeIkTree.def("GetEffectorCount", &Tree::GetNumEffector);
	classTreeIkTree.def("GetJointCount", &Tree::GetNumJoint);
	classTreeIkTree.def("Compute", &Tree::Compute);
	classTreeIkTree.def("Init", &Tree::Init);
	classTreeIkTree.def("UnFreeze", &Tree::UnFreeze);
	classTreeIkTree.def("Print", &Tree::Print);
	classTreeIkTree.def("InsertRoot", &Tree::InsertRoot);
	classTreeIkTree.def("InsertLeftChild", &Tree::InsertLeftChild);
	classTreeIkTree.def("InsertRightSibling", &Tree::InsertRightSibling);
	classTreeIkTree.def("GetJoint", &Tree::GetJoint, luabind::shared_from_this_policy<0> {});
	classTreeIkTree.def("GetEffector", &Tree::GetEffector, luabind::shared_from_this_policy<0> {});
	classTreeIkTree.def("GetEffectorPosition", static_cast<Vector3 (*)(lua_State *, Tree &, uint32_t)>([](lua_State *l, Tree &tree, uint32_t nodeIdx) {
		auto &pos = tree.GetEffectorPosition(nodeIdx);
		return Vector3(pos.x, pos.y, pos.z);
	}));
	classTreeIkTree.def("GetRoot", &Tree::GetRoot, luabind::shared_from_this_policy<0> {});
	classTreeIkTree.def("GetSuccessor", &Tree::GetSuccessor, luabind::shared_from_this_policy<0> {});
	classTreeIkTree.def("GetParent", &Tree::GetParent, luabind::shared_from_this_policy<0> {});

	auto classTreeIkTreeNode = luabind::class_<Node>("Node");
	classTreeIkTreeNode.add_static_constant("PURPOSE_JOINT", JOINT);
	classTreeIkTreeNode.add_static_constant("PURPOSE_EFFECTOR", EFFECTOR);
	classTreeIkTreeNode.scope[luabind::def("Create",
	  static_cast<std::shared_ptr<Node> (*)(lua_State *, const Vector3 &, const Vector3 &, uint32_t, double, double, double)>([](lua_State *l, const Vector3 &origin, const Vector3 &rotAxis, uint32_t purpose, double minTheta, double maxTheta, double restAngle) -> std::shared_ptr<Node> {
		  return std::make_shared<Node>(VectorR3(origin.x, origin.y, origin.z), VectorR3(rotAxis.x, rotAxis.y, rotAxis.z), 0.0, static_cast<Purpose>(purpose), minTheta, maxTheta, restAngle);
	  }))];
	classTreeIkTreeNode.scope[luabind::def("Create", static_cast<std::shared_ptr<Node> (*)(lua_State *, const Vector3 &, const Vector3 &, uint32_t)>([](lua_State *l, const Vector3 &origin, const Vector3 &rotAxis, uint32_t purpose) -> std::shared_ptr<Node> {
		return std::make_shared<Node>(VectorR3(origin.x, origin.y, origin.z), VectorR3(rotAxis.x, rotAxis.y, rotAxis.z), 0.0, static_cast<Purpose>(purpose));
	}))];
	classTreeIkTreeNode.def("GetLocalTransform", static_cast<umath::Transform (*)(lua_State *, Node &)>([](lua_State *l, Node &node) {
		umath::Transform t {};
		util::ik::get_local_transform(node, t);
		return t;
	}));
	classTreeIkTreeNode.def("PrintNode", &Node::PrintNode);
	classTreeIkTreeNode.def("GetRotationAxis", static_cast<Vector3 (*)(lua_State *, Node &)>([](lua_State *l, Node &node) {
		auto &v = node.v;
		return Vector3(v.x, v.y, v.z);
	}));
	classTreeIkTreeNode.def("SetRotationAxis", static_cast<void (*)(lua_State *, Node &, const Vector3 &)>([](lua_State *l, Node &node, const Vector3 &axis) {
		auto &v = node.v;
		v = VectorR3(axis.x, axis.y, axis.z);
	}));
	classTreeIkTreeNode.def("GetLeftChildNode", static_cast<std::shared_ptr<Node> (*)(lua_State *, Node &)>([](lua_State *l, Node &node) -> std::shared_ptr<Node> {
		auto *left = node.left;
		if(left == nullptr)
			return nullptr;
		return left->shared_from_this();
	}));
	classTreeIkTreeNode.def("GetRightChildNode", static_cast<std::shared_ptr<Node> (*)(lua_State *, Node &)>([](lua_State *l, Node &node) -> std::shared_ptr<Node> {
		auto *right = node.right;
		if(right == nullptr)
			return nullptr;
		return right->shared_from_this();
	}));
	classTreeIkTreeNode.def("InitNode", &Node::InitNode);
	classTreeIkTreeNode.def("GetAttach", static_cast<Vector3 (*)(lua_State *, Node &)>([](lua_State *l, Node &node) {
		auto &r = node.GetAttach();
		return Vector3(r.x, r.y, r.z);
	}));
	classTreeIkTreeNode.def("SetAttach", static_cast<void (*)(lua_State *, Node &, const Vector3 &)>([](lua_State *l, Node &node, const Vector3 &attach) { node.attach = VectorR3(attach.x, attach.y, attach.z); }));
	classTreeIkTreeNode.def("GetRelativePosition", static_cast<Vector3 (*)(lua_State *, Node &)>([](lua_State *l, Node &node) {
		auto &r = node.r;
		return Vector3(r.x, r.y, r.z);
	}));
	classTreeIkTreeNode.def("GetTheta", &Node::GetTheta);
	classTreeIkTreeNode.def("AddToTheta", static_cast<double (*)(lua_State *, Node &, double)>([](lua_State *l, Node &node, double delta) { return node.AddToTheta(delta); }));
	classTreeIkTreeNode.def("UpdateTheta", static_cast<double (*)(lua_State *, Node &, double)>([](lua_State *l, Node &node, double delta) { return node.UpdateTheta(delta); }));
	classTreeIkTreeNode.def("GetS", static_cast<Vector3 (*)(lua_State *, Node &)>([](lua_State *l, Node &node) {
		auto &s = node.GetS();
		return Vector3(s.x, s.y, s.z);
	}));
	classTreeIkTreeNode.def("GetW", static_cast<Vector3 (*)(lua_State *, Node &)>([](lua_State *l, Node &node) {
		auto &w = node.GetW();
		return Vector3(w.x, w.y, w.z);
	}));
	classTreeIkTreeNode.def("GetMinTheta", &Node::GetMinTheta);
	classTreeIkTreeNode.def("GetMaxTheta", &Node::GetMaxTheta);
	classTreeIkTreeNode.def("GetRestAngle", &Node::GetRestAngle);
	classTreeIkTreeNode.def("SetTheta", &Node::SetTheta);
	classTreeIkTreeNode.def("ComputeS", &Node::ComputeS);
	classTreeIkTreeNode.def("ComputeW", &Node::ComputeW);
	classTreeIkTreeNode.def("IsEffector", &Node::IsEffector);
	classTreeIkTreeNode.def("IsJoint", &Node::IsJoint);
	classTreeIkTreeNode.def("GetEffectorIndex", &Node::GetEffectorNum);
	classTreeIkTreeNode.def("GetJointIndex", &Node::GetJointNum);
	classTreeIkTreeNode.def("IsFrozen", &Node::IsFrozen);
	classTreeIkTreeNode.def("Freeze", &Node::Freeze);
	classTreeIkTreeNode.def("UnFreeze", &Node::UnFreeze);

	auto classIkJacobian = luabind::class_<Jacobian>("IKJacobian");
	classIkJacobian.scope[luabind::def("Create", static_cast<std::shared_ptr<Jacobian> (*)(lua_State *, Tree &)>([](lua_State *l, Tree &tree) { return std::make_shared<Jacobian>(&tree); }))];
	classIkJacobian.def("ComputeJacobian", static_cast<void (*)(lua_State *, Jacobian &, const luabind::tableT<Vector3> &)>([](lua_State *l, Jacobian &jacobian, const luabind::tableT<Vector3> &vTargets) {
		auto numTargets = Lua::GetObjectLength(l, vTargets);
		std::vector<VectorR3> targets;
		targets.reserve(numTargets);
		for(luabind::iterator it {vTargets}, end; it != end; ++it) {
			auto v = luabind::object_cast<Vector3>(*it);
			targets.push_back(VectorR3(v.x, v.y, v.z));
		}
		jacobian.ComputeJacobian(targets.data());
	}));
	classIkJacobian.def("SetJendActive", &Jacobian::SetJendActive);
	classIkJacobian.def("SetJtargetActive", &Jacobian::SetJtargetActive);
	//classIkJacobian.def("SetJendTrans",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.SetJendTrans();
	//}));
	//classIkJacobian.def("SetDeltaS",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian,const Vector3 &s) {
	//	jacobian.SetDeltaS();
	//}));
	classIkJacobian.def("CalcDeltaThetas", &Jacobian::CalcDeltaThetas);
	classIkJacobian.def("ZeroDeltaThetas", &Jacobian::ZeroDeltaThetas);
	classIkJacobian.def("CalcDeltaThetasTranspose", &Jacobian::CalcDeltaThetasTranspose);
	classIkJacobian.def("CalcDeltaThetasPseudoinverse", &Jacobian::CalcDeltaThetasPseudoinverse);
	classIkJacobian.def("CalcDeltaThetasDLS", &Jacobian::CalcDeltaThetasDLS);
	//classIkJacobian.def("CalcDeltaThetasDLS2",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.CalcDeltaThetasDLS2();
	//}));
	classIkJacobian.def("CalcDeltaThetasDLSwithSVD", &Jacobian::CalcDeltaThetasDLSwithSVD);
	classIkJacobian.def("CalcDeltaThetasSDLS", &Jacobian::CalcDeltaThetasSDLS);
	//classIkJacobian.def("CalcDeltaThetasDLSwithNullspace",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.CalcDeltaThetasDLSwithNullspace();
	//}));
	classIkJacobian.def("UpdateThetas", &Jacobian::UpdateThetas);
	classIkJacobian.def("UpdateThetaDot", &Jacobian::UpdateThetaDot);
	//classIkJacobian.def("UpdateErrorArray",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.UpdateErrorArray();
	//}));
	//classIkJacobian.def("GetErrorArray",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
	//	jacobian.GetErrorArray();
	//}));
	classIkJacobian.def("UpdatedSClampValue", static_cast<void (*)(lua_State *, Jacobian &, const luabind::tableT<Vector3> &)>([](lua_State *l, Jacobian &jacobian, const luabind::tableT<Vector3> &vTargets) {
		auto numTargets = Lua::GetObjectLength(l, vTargets);
		std::vector<VectorR3> targets;
		targets.reserve(numTargets);
		for(luabind::iterator it {vTargets}, end; it != end; ++it) {
			auto v = luabind::object_cast<Vector3>(*it);
			targets.push_back(VectorR3(v.x, v.y, v.z));
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
	classIkJacobian.def("Reset", &Jacobian::Reset);
	/*classIkJacobian.def("CompareErrors",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CompareErrors();
	}));
	classIkJacobian.def("CountErrors",static_cast<void(*)(lua_State*,Jacobian&)>([](lua_State *l,Jacobian &jacobian) {
		jacobian.CountErrors();
	}));*/
	classIkJacobian.def("GetRowCount", &Jacobian::GetNumRows);
	classIkJacobian.def("GetColumnCount", &Jacobian::GetNumCols);
	physMod[classIkJacobian];

	classTreeIkTree.scope[classTreeIkTreeNode];
	physMod[classTreeIkTree];

	auto classIkController = luabind::class_<IKController>("IKController");
	classIkController.def("GetEffectorName", &IKController::GetEffectorName);
	classIkController.def("GetChainLength", &IKController::GetChainLength);
	classIkController.def("GetType", &IKController::GetType);
	classIkController.def("SetEffectorName", &IKController::SetEffectorName);
	classIkController.def("SetChainLength", &IKController::SetChainLength);
	classIkController.def("SetType", &IKController::SetType);
	classIkController.def("SetMethod", &IKController::SetMethod);
	classIkController.def("GetMethod", &IKController::GetMethod);
	classIkController.def("GetKeyValues", static_cast<std::unordered_map<std::string, std::string> &(IKController::*)()>(&IKController::GetKeyValues));
	classIkController.def("SetKeyValues",
	  static_cast<void (*)(lua_State *, IKController &, std::unordered_map<std::string, std::string> &&)>([](lua_State *l, IKController &ikController, std::unordered_map<std::string, std::string> &&keyValues) { ikController.GetKeyValues() = std::move(keyValues); }));
	classIkController.def("SetKeyValue", static_cast<void (*)(lua_State *, IKController &, const std::string &, const std::string &)>([](lua_State *l, IKController &ikController, const std::string &key, const std::string &value) {
		auto &ikKeyValues = ikController.GetKeyValues();
		ikKeyValues[key] = value;
	}));
	classIkController.def("GetKeyValue", static_cast<std::optional<std::string> (*)(lua_State *, IKController &, const std::string &)>([](lua_State *l, IKController &ikController, const std::string &key) -> std::optional<std::string> {
		auto &ikKeyValues = ikController.GetKeyValues();
		auto it = ikKeyValues.find(key);
		if(it == ikKeyValues.end())
			return {};
		return it->second;
	}));
	classIkController.add_static_constant("METHOD_SELECTIVELY_DAMPED_LEAST_SQUARE", umath::to_integral(util::ik::Method::SelectivelyDampedLeastSquare));
	classIkController.add_static_constant("METHOD_DAMPED_LEAST_SQUARES", umath::to_integral(util::ik::Method::DampedLeastSquares));
	classIkController.add_static_constant("METHOD_DAMPED_LEAST_SQUARES_WITH_SINGULAR_VALUE_DECOMPOSITION", umath::to_integral(util::ik::Method::DampedLeastSquaresWithSingularValueDecomposition));
	classIkController.add_static_constant("METHOD_PSEUDOINVERSE", umath::to_integral(util::ik::Method::Pseudoinverse));
	classIkController.add_static_constant("METHOD_JACOBIAN_TRANSPOSE", umath::to_integral(util::ik::Method::JacobianTranspose));
	classIkController.add_static_constant("METHOD_DEFAULT", umath::to_integral(util::ik::Method::Default));
	physMod[classIkController];

	auto classDef = luabind::class_<::PhysSoftBodyInfo>("SoftBodyInfo");
	Lua::PhysSoftBodyInfo::register_class(l, classDef);
	physMod[classDef];

	Lua::PhysConstraint::register_class(l, physMod);
	Lua::PhysCollisionObj::register_class(l, physMod);
	Lua::PhysObj::register_class(l, physMod);
	Lua::PhysContact::register_class(l, physMod);
	Lua::PhysShape::register_class(l, physMod);
	Lua::PhysKinematicCharacterController::register_class(l, physMod);
}
Lua::var<bool, luabind::tableT<TraceResult>, TraceResult> Lua::physenv::raycast(lua_State *l, Game &game, const ::TraceData &traceData)
{
	std::vector<TraceResult> res;
	auto r = game.RayCast(traceData, &res);
	if(res.empty() || (r == false && (traceData.HasFlag(RayCastFlags::ReportAllResults) == false || res.size() == 1)))
		return luabind::object {l, r};
	if(!traceData.HasFlag(RayCastFlags::ReportAllResults))
		return luabind::object {l, static_cast<TraceResult>(res.back())};
	auto table = luabind::newtable(l);
	for(size_t i = 0; i < res.size(); i++)
		table[i + 1] = res[i];
	return table;
}
Lua::var<bool, luabind::tableT<TraceResult>, TraceResult> Lua::physenv::sweep(lua_State *l, Game &game, const ::TraceData &traceData)
{
	std::vector<TraceResult> res;
	auto r = game.Sweep(traceData, &res);
	if(res.empty() || (r == false && (traceData.HasFlag(RayCastFlags::ReportAllResults) == false || res.size() == 1)))
		return luabind::object {l, r};
	if(!traceData.HasFlag(RayCastFlags::ReportAllResults))
		return luabind::object {l, static_cast<TraceResult>(res.back())};
	auto table = luabind::newtable(l);
	for(size_t i = 0; i < res.size(); i++)
		table[i + 1] = res[i];
	return table;
}
Lua::var<bool, luabind::tableT<TraceResult>, TraceResult> Lua::physenv::overlap(lua_State *l, Game &game, const ::TraceData &traceData)
{
	std::vector<TraceResult> res;
	auto r = game.Overlap(traceData, &res);
	if(res.empty() || (r == false && (traceData.HasFlag(RayCastFlags::ReportAllResults) == false || res.size() == 1)))
		return luabind::object {l, r};
	if(!traceData.HasFlag(RayCastFlags::ReportAllResults))
		return luabind::object {l, static_cast<TraceResult>(res.back())};
	auto table = luabind::newtable(l);
	for(size_t i = 0; i < res.size(); i++)
		table[i + 1] = res[i];
	return table;
}
std::shared_ptr<pragma::physics::IConvexHullShape> Lua::physenv::create_convex_hull_shape(pragma::physics::IEnvironment *env, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	return env->CreateConvexHullShape(material);
}

std::shared_ptr<pragma::physics::ITriangleShape> Lua::physenv::create_triangle_shape(pragma::physics::IEnvironment *env, const std::vector<Vector3> &verts, const std::vector<uint16_t> &tris, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	auto shape = env->CreateTriangleShape(material);
	if(!shape)
		return nullptr;
	shape->ReserveTriangles(tris.size() / 3);
	for(auto i = decltype(tris.size()) {0u}; i < tris.size(); i += 3)
		shape->AddTriangle(verts[tris[i]], verts[tris[i + 1]], verts[tris[i + 2]]);
	shape->Build();
	return shape;
}
std::shared_ptr<pragma::physics::IConvexShape> Lua::physenv::create_convex_shape(pragma::physics::IEnvironment *env, const std::vector<Vector3> &verts, const std::vector<uint16_t> &tris, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	auto shape = env->CreateConvexHullShape(material);
	if(!shape)
		return nullptr;
	shape->ReservePoints(verts.size());
	for(auto &v : verts)
		shape->AddPoint(v);
	shape->ReserveTriangles(tris.size() / 3);
	for(auto i = decltype(tris.size()) {0u}; i < tris.size(); i += 3)
		shape->AddTriangle(tris[i], tris[i + 1], tris[i + 2]);
	shape->Build();
	return shape;
}

std::shared_ptr<pragma::physics::IConvexShape> Lua::physenv::create_convex_shape(pragma::physics::IEnvironment *env, const std::vector<Vector3> &verts, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	auto shape = env->CreateConvexHullShape(material);
	if(!shape)
		return nullptr;
	shape->ReservePoints(verts.size());
	for(auto &v : verts)
		shape->AddPoint(v);
	shape->Build();
	return shape;
}

std::shared_ptr<pragma::physics::IConvexShape> Lua::physenv::create_box_shape(pragma::physics::IEnvironment *env, const Vector3 &halfExtents, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	return env->CreateBoxShape(halfExtents, material);
}

std::shared_ptr<pragma::physics::IConvexShape> Lua::physenv::create_capsule_shape(pragma::physics::IEnvironment *env, float halfWidth, float halfHeight, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	return env->CreateCapsuleShape(halfWidth, halfHeight, material);
}

std::shared_ptr<pragma::physics::IConvexShape> Lua::physenv::create_sphere_shape(pragma::physics::IEnvironment *env, float radius, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	return env->CreateSphereShape(radius, material);
}

std::shared_ptr<pragma::physics::IConvexShape> Lua::physenv::create_cylinder_shape(pragma::physics::IEnvironment *env, float radius, float height, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	return env->CreateCylinderShape(radius, height, material);
}

std::shared_ptr<pragma::physics::ICompoundShape> Lua::physenv::create_compound_shape(pragma::physics::IEnvironment *env, std::vector<pragma::physics::IShape *> &shapes)
{
	if(!env)
		return nullptr;
	return env->CreateCompoundShape(shapes);
}

std::shared_ptr<pragma::physics::IShape> Lua::physenv::create_heightfield_terrain_shape(pragma::physics::IEnvironment *env, float width, float length, float maxHeight, float upAxis, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	return env->CreateHeightfieldTerrainShape(width, length, maxHeight, upAxis, material);
}

util::TSharedHandle<pragma::physics::IRigidBody> Lua::physenv::create_rigid_body(pragma::physics::IEnvironment *env, pragma::physics::IShape &shape, bool dynamic)
{
	if(!env)
		return nullptr;
	return env->CreateRigidBody(shape, dynamic);
}

util::TSharedHandle<pragma::physics::IGhostObject> Lua::physenv::create_ghost_object(pragma::physics::IEnvironment *env, pragma::physics::IShape &shape)
{
	if(!env)
		return nullptr;
	return env->CreateGhostObject(shape);
}

util::TSharedHandle<pragma::physics::ICollisionObject> Lua::physenv::create_plane(pragma::physics::IEnvironment *env, const Vector3 &n, double d, pragma::physics::IMaterial &material)
{
	if(!env)
		return nullptr;
	return env->CreatePlane(n, d, material);
}

util::TSharedHandle<pragma::physics::IFixedConstraint> Lua::physenv::create_fixed_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Quat &rotB)
{
	if(!env)
		return nullptr;
	return env->CreateFixedConstraint(bodyA, pivotA, rotA, bodyB, pivotB, rotB);
}

util::TSharedHandle<pragma::physics::IBallSocketConstraint> Lua::physenv::create_ball_socket_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB)
{
	if(!env)
		return nullptr;
	return env->CreateBallSocketConstraint(bodyA, pivotA, bodyB, pivotB);
}

util::TSharedHandle<pragma::physics::IHingeConstraint> Lua::physenv::create_hinge_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Vector3 &axis)
{
	if(!env)
		return nullptr;
	return env->CreateHingeConstraint(bodyA, pivotA, bodyB, pivotB, axis);
}

util::TSharedHandle<pragma::physics::ISliderConstraint> Lua::physenv::create_slider_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Quat &rotB)
{
	if(!env)
		return nullptr;
	return env->CreateSliderConstraint(bodyA, pivotA, rotA, bodyB, pivotB, rotB);
}

util::TSharedHandle<pragma::physics::IConeTwistConstraint> Lua::physenv::create_cone_twist_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB,
  const Quat &rotB)
{
	if(!env)
		return nullptr;
	return env->CreateConeTwistConstraint(bodyA, pivotA, rotA, bodyB, pivotB, rotB);
}

util::TSharedHandle<pragma::physics::IDoFConstraint> Lua::physenv::create_DoF_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB, const Quat &rotB)
{
	if(!env)
		return nullptr;
	return env->CreateDoFConstraint(bodyA, pivotA, rotA, bodyB, pivotB, rotB);
}

util::TSharedHandle<pragma::physics::IDoFSpringConstraint> Lua::physenv::create_dof_spring_constraint(pragma::physics::IEnvironment *env, pragma::physics::IRigidBody &bodyA, const Vector3 &pivotA, const Quat &rotA, pragma::physics::IRigidBody &bodyB, const Vector3 &pivotB,
  const Quat &rotB)
{
	if(!env)
		return nullptr;
	return env->CreateDoFSpringConstraint(bodyA, pivotA, rotA, bodyB, pivotB, rotB);
}

SurfaceMaterial *Lua::physenv::create_surface_material(Game &game, const std::string &name, float friction, float restitution) { return &game.CreateSurfaceMaterial(name, friction, restitution); }

util::TSharedHandle<pragma::physics::IController> Lua::physenv::create_box_controller(pragma::physics::IEnvironment *env, const Vector3 &halfExtents, float stepHeight, float slopeLimit, const umath::Transform &startTransform)
{
	if(!env)
		return nullptr;
	return env->CreateBoxController(halfExtents, stepHeight, slopeLimit, startTransform);
}
util::TSharedHandle<pragma::physics::IController> Lua::physenv::create_capsule_controller(pragma::physics::IEnvironment *env, float halfWidth, float halfHeight, float stepHeight, float slopeLimit, const umath::Transform &startTransform)
{
	if(!env)
		return nullptr;
	return env->CreateCapsuleController(halfWidth, halfHeight, stepHeight, slopeLimit, startTransform);
}

std::shared_ptr<pragma::physics::IMaterial> Lua::physenv::create_material(pragma::physics::IEnvironment *env, float staticFriction, float dynamicFriction, float restitution)
{
	if(!env)
		return nullptr;
	return env->CreateMaterial(staticFriction, dynamicFriction, restitution);
}
luabind::tableT<SurfaceMaterial> Lua::physenv::get_surface_materials(lua_State *l, Game &game)
{
	auto &mats = game.GetSurfaceMaterials();
	auto t = luabind::newtable(l);
	for(UInt i = 0; i < mats.size(); i++)
		t[i + 1] = &mats[i];
	return t;
}

void Lua::physenv::create_character_controller(lua_State *)
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

	Lua::Push<PhysKinematicCharacterController*>(l,controller);
	return 0;*/
}

Vector3 Lua::physenv::calc_torque_from_angular_velocity(const Vector3 &angVel, const Mat3 &invInertiaTensor, float dt)
{
	auto torque = angVel / static_cast<float>(dt);
	return glm::inverse(invInertiaTensor) * torque;
}
Vector3 Lua::physenv::calc_angular_velocity_from_torque(const Vector3 &torque, const Mat3 &invInertiaTensor, float dt) { return torque * invInertiaTensor * static_cast<float>(dt); }
Vector3 Lua::physenv::calc_force_from_linear_velocity(const Vector3 &linVel, float mass, float dt) { return (static_cast<float>(mass) * linVel) / static_cast<float>(dt); }
Vector3 Lua::physenv::calc_linear_velocity_from_force(const Vector3 &force, float mass, float dt) { return (force * static_cast<float>(dt)) / static_cast<float>(mass); }
