/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/ik.hpp"
#include "pragma/lua/luaapi.h"
#include "pragma/lua/custom_constructor.hpp"
#include "pragma/lua/converters/pair_converter_t.hpp"
#include "pragma/lua/ostream_operator_alias.hpp"
#include <luainterface.hpp>
#include <luabind/copy_policy.hpp>
#include <bepuik/joint/IKJoint.hpp>
#include <bepuik/joint/IKBallSocketJoint.hpp>
namespace Lua::ik {
	void register_library(Lua::Interface &lua);
};

static luabind::object joint_to_lua_object(lua_State *l, pragma::ik::IJoint &joint)
{
	switch(joint.GetJointType()) {
	case pragma::ik::JointType::DistanceJoint:
		return luabind::object {l, &static_cast<pragma::ik::DistanceJoint &>(joint)};
	case pragma::ik::JointType::BallSocketJoint:
		return luabind::object {l, &static_cast<pragma::ik::BallSocketJoint &>(joint)};
	case pragma::ik::JointType::AngularJoint:
		return luabind::object {l, &static_cast<pragma::ik::AngularJoint &>(joint)};
	case pragma::ik::JointType::PointOnLineJoint:
		return luabind::object {l, &static_cast<pragma::ik::PointOnLineJoint &>(joint)};
	case pragma::ik::JointType::RevoluteJoint:
		return luabind::object {l, &static_cast<pragma::ik::RevoluteJoint &>(joint)};
	case pragma::ik::JointType::SwingLimit:
		return luabind::object {l, &static_cast<pragma::ik::SwingLimit &>(joint)};
	case pragma::ik::JointType::EllipseSwingLimit:
		return luabind::object {l, &static_cast<pragma::ik::EllipseSwingLimit &>(joint)};
	case pragma::ik::JointType::LinearAxisLimit:
		return luabind::object {l, &static_cast<pragma::ik::LinearAxisLimit &>(joint)};
	case pragma::ik::JointType::TwistJoint:
		return luabind::object {l, &static_cast<pragma::ik::TwistJoint &>(joint)};
	case pragma::ik::JointType::TwistLimit:
		return luabind::object {l, &static_cast<pragma::ik::TwistLimit &>(joint)};
	case pragma::ik::JointType::SwivelHingeJoint:
		return luabind::object {l, &static_cast<pragma::ik::SwivelHingeJoint &>(joint)};
	}
	static_assert(umath::to_integral(pragma::ik::JointType::Count) == 11);
	return Lua::nil;
}

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, Bone);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, IJoint);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, BallSocketJoint);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, AngularJoint);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, PointOnLineJoint);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, RevoluteJoint);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, SwingLimit);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, EllipseSwingLimit);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, LinearAxisLimit);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, TwistJoint);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, TwistLimit);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, SwivelHingeJoint);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, IControl);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, ILinearMotorControl);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, DragControl);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, AngularPlaneControl);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, StateControl);
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma::ik, Solver);
#endif

void Lua::ik::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();

	const auto *libName = "ik";
	auto &modIk = lua.RegisterLibrary(libName);

	auto classBone = luabind::class_<pragma::ik::Bone>("Bone");
	classBone.def(luabind::const_self == luabind::const_self);
	classBone.def(luabind::tostring(luabind::self));
	classBone.def("GetPos", &pragma::ik::Bone::GetPos);
	classBone.def("SetPos", &pragma::ik::Bone::SetPos);
	classBone.def("GetRot", &pragma::ik::Bone::GetRot);
	classBone.def("SetRot", &pragma::ik::Bone::SetRot);
	classBone.def("SetPinned", &pragma::ik::Bone::SetPinned);
	classBone.def("IsPinned", &pragma::ik::Bone::IsPinned);
	classBone.def("SetName", &pragma::ik::Bone::SetName);
	classBone.def("GetName", &pragma::ik::Bone::GetName);
	classBone.def("GetRadius", &pragma::ik::Bone::GetRadius);
	classBone.def("GetLength", &pragma::ik::Bone::GetLength);
	classBone.def("GetMass", &pragma::ik::Bone::GetMass);
	classBone.def("GetOriginalPose", &pragma::ik::Bone::GetOriginalPose, luabind::copy_policy<0> {});
	modIk[classBone];

	auto classJoint = luabind::class_<pragma::ik::IJoint>("Joint");
	classJoint.def(luabind::tostring(luabind::self));
	classJoint.add_static_constant("TYPE_DISTANCE_JOINT", umath::to_integral(pragma::ik::JointType::DistanceJoint));
	classJoint.add_static_constant("TYPE_BALL_SOCKET_JOINT", umath::to_integral(pragma::ik::JointType::BallSocketJoint));
	classJoint.add_static_constant("TYPE_ANGULAR_JOINT", umath::to_integral(pragma::ik::JointType::AngularJoint));
	classJoint.add_static_constant("TYPE_POINT_ON_LINE_JOINT", umath::to_integral(pragma::ik::JointType::PointOnLineJoint));
	classJoint.add_static_constant("TYPE_REVOLUTE_JOINT", umath::to_integral(pragma::ik::JointType::RevoluteJoint));
	classJoint.add_static_constant("TYPE_SWING_LIMIT", umath::to_integral(pragma::ik::JointType::SwingLimit));
	classJoint.add_static_constant("TYPE_ELLIPSE_SWING_LIMIT", umath::to_integral(pragma::ik::JointType::EllipseSwingLimit));
	classJoint.add_static_constant("TYPE_LINEAR_AXIS_LIMIT", umath::to_integral(pragma::ik::JointType::LinearAxisLimit));
	classJoint.add_static_constant("TYPE_TWIST_JOINT", umath::to_integral(pragma::ik::JointType::TwistJoint));
	classJoint.add_static_constant("TYPE_TWIST_LIMIT", umath::to_integral(pragma::ik::JointType::TwistLimit));
	classJoint.add_static_constant("TYPE_SWIVEL_HINGE_JOINT", umath::to_integral(pragma::ik::JointType::SwivelHingeJoint));
	classJoint.add_static_constant("TYPE_COUNT", umath::to_integral(pragma::ik::JointType::Count));
	classJoint.add_static_constant("TYPE_INVALID", umath::to_integral(pragma::ik::JointType::Invalid));
	static_assert(umath::to_integral(pragma::ik::JointType::Count) == 11);
	classJoint.def("SetRigidity", &pragma::ik::IJoint::SetRigidity);
	classJoint.def("GetRigidity", &pragma::ik::IJoint::GetRigidity);
	classJoint.def("GetConnectionA", static_cast<pragma::ik::Bone &(pragma::ik::IJoint::*)()>(&pragma::ik::IJoint::GetConnectionA));
	classJoint.def("GetConnectionB", static_cast<pragma::ik::Bone &(pragma::ik::IJoint::*)()>(&pragma::ik::IJoint::GetConnectionB));
	classJoint.def("GetType", &pragma::ik::IJoint::GetJointType);
	modIk[classJoint];

	auto classDistanceJoint = luabind::class_<pragma::ik::DistanceJoint, pragma::ik::IJoint>("DistanceJoint");
	classDistanceJoint.def(luabind::tostring(luabind::self));
	modIk[classDistanceJoint];

	auto classBallSocketJoint = luabind::class_<pragma::ik::BallSocketJoint, pragma::ik::IJoint>("BallSocketJoint");
	classBallSocketJoint.def(luabind::tostring(luabind::self));
	classBallSocketJoint.def("GetAnchor", &pragma::ik::BallSocketJoint::GetAnchor, luabind::copy_policy<0> {});
	classBallSocketJoint.def("GetOffsetA", &pragma::ik::BallSocketJoint::GetOffsetA);
	classBallSocketJoint.def("GetOffsetB", &pragma::ik::BallSocketJoint::GetOffsetB);
	modIk[classBallSocketJoint];

	auto classAngularJoint = luabind::class_<pragma::ik::AngularJoint, pragma::ik::IJoint>("AngularJoint");
	classAngularJoint.def(luabind::tostring(luabind::self));
	modIk[classAngularJoint];

	auto classPointOnLineJoint = luabind::class_<pragma::ik::PointOnLineJoint, pragma::ik::IJoint>("PointOnLineJoint");
	classPointOnLineJoint.def(luabind::tostring(luabind::self));
	classPointOnLineJoint.def("GetLineAnchor", &pragma::ik::PointOnLineJoint::GetLineAnchor, luabind::copy_policy<0> {});
	classPointOnLineJoint.def("GetLineDirection", &pragma::ik::PointOnLineJoint::GetLineDirection, luabind::copy_policy<0> {});
	classPointOnLineJoint.def("GetAnchorB", &pragma::ik::PointOnLineJoint::GetAnchorB, luabind::copy_policy<0> {});
	modIk[classPointOnLineJoint];

	auto classRevoluteJoint = luabind::class_<pragma::ik::RevoluteJoint, pragma::ik::IJoint>("RevoluteJoint");
	classRevoluteJoint.def(luabind::tostring(luabind::self));
	classRevoluteJoint.def("GetFreeAxis", &pragma::ik::RevoluteJoint::GetFreeAxis, luabind::copy_policy<0> {});
	modIk[classRevoluteJoint];

	auto classSwingLimit = luabind::class_<pragma::ik::SwingLimit, pragma::ik::IJoint>("SwingLimit");
	classSwingLimit.def(luabind::tostring(luabind::self));
	classSwingLimit.def("GetAxisA", &pragma::ik::SwingLimit::GetAxisA);
	classSwingLimit.def("GetAxisB", &pragma::ik::SwingLimit::GetAxisB);
	classSwingLimit.def("GetMaxAngle", &pragma::ik::SwingLimit::GetMaxAngle);
	modIk[classSwingLimit];

	auto classEllipseSwingLimit = luabind::class_<pragma::ik::EllipseSwingLimit, pragma::ik::IJoint>("EllipseSwingLimit");
	classEllipseSwingLimit.def(luabind::tostring(luabind::self));
	classEllipseSwingLimit.def("GetAxisA", &pragma::ik::EllipseSwingLimit::GetAxisA, luabind::copy_policy<0> {});
	classEllipseSwingLimit.def("GetAxisB", &pragma::ik::EllipseSwingLimit::GetAxisB, luabind::copy_policy<0> {});
	classEllipseSwingLimit.def("GetXAxis", &pragma::ik::EllipseSwingLimit::GetXAxis, luabind::copy_policy<0> {});
	classEllipseSwingLimit.def("GetMaxAngleX", &pragma::ik::EllipseSwingLimit::GetMaxAngleX);
	classEllipseSwingLimit.def("GetMaxAngleY", &pragma::ik::EllipseSwingLimit::GetMaxAngleY);
	modIk[classEllipseSwingLimit];

	auto classLinearAxisLimit = luabind::class_<pragma::ik::LinearAxisLimit, pragma::ik::IJoint>("LinearAxisLimit");
	classLinearAxisLimit.def(luabind::tostring(luabind::self));
	classLinearAxisLimit.def("GetLineAnchor", &pragma::ik::LinearAxisLimit::GetLineAnchor, luabind::copy_policy<0> {});
	classLinearAxisLimit.def("GetLineDirection", &pragma::ik::LinearAxisLimit::GetLineDirection, luabind::copy_policy<0> {});
	classLinearAxisLimit.def("GetAnchorB", &pragma::ik::LinearAxisLimit::GetAnchorB, luabind::copy_policy<0> {});
	classLinearAxisLimit.def("GetMinimumDistance", &pragma::ik::LinearAxisLimit::GetMinimumDistance);
	classLinearAxisLimit.def("GetMaximumDistance", &pragma::ik::LinearAxisLimit::GetMaximumDistance);
	modIk[classLinearAxisLimit];

	auto classTwistJoint = luabind::class_<pragma::ik::TwistJoint, pragma::ik::IJoint>("TwistJoint");
	classTwistJoint.def(luabind::tostring(luabind::self));
	classTwistJoint.def("GetAxisA", &pragma::ik::TwistJoint::GetAxisA, luabind::copy_policy<0> {});
	classTwistJoint.def("GetAxisB", &pragma::ik::TwistJoint::GetAxisB, luabind::copy_policy<0> {});
	modIk[classTwistJoint];

	auto classTwistLimit = luabind::class_<pragma::ik::TwistLimit, pragma::ik::IJoint>("TwistLimit");
	classTwistLimit.def(luabind::tostring(luabind::self));
	classTwistLimit.def("SetMeasurementAxisA", &pragma::ik::TwistLimit::SetMeasurementAxisA);
	classTwistLimit.def("GetMeasurementAxisA", &pragma::ik::TwistLimit::GetMeasurementAxisA);
	classTwistLimit.def("SetMeasurementAxisB", &pragma::ik::TwistLimit::SetMeasurementAxisB);
	classTwistLimit.def("GetMeasurementAxisB", &pragma::ik::TwistLimit::GetMeasurementAxisB);
	classTwistLimit.def("GetMaxAngle", &pragma::ik::TwistLimit::GetMaxAngle);
	classTwistLimit.def("GetAxisA", &pragma::ik::TwistLimit::GetAxisA, luabind::copy_policy<0> {});
	classTwistLimit.def("GetAxisB", &pragma::ik::TwistLimit::GetAxisB, luabind::copy_policy<0> {});
	modIk[classTwistLimit];

	auto classSwivelHingeJoint = luabind::class_<pragma::ik::SwivelHingeJoint, pragma::ik::IJoint>("SwivelHingeJoint");
	classSwivelHingeJoint.def(luabind::tostring(luabind::self));
	classSwivelHingeJoint.def("GetWorldHingeAxis", &pragma::ik::SwivelHingeJoint::GetWorldHingeAxis, luabind::copy_policy<0> {});
	classSwivelHingeJoint.def("GetWorldTwistAxis", &pragma::ik::SwivelHingeJoint::GetWorldTwistAxis, luabind::copy_policy<0> {});
	modIk[classSwivelHingeJoint];

	auto classControl = luabind::class_<pragma::ik::IControl>("Control");
	classControl.add_static_constant("TYPE_DRAG", umath::to_integral(pragma::ik::ControlType::Drag));
	classControl.add_static_constant("TYPE_ANGULAR_PLANE", umath::to_integral(pragma::ik::ControlType::AngularPlane));
	classControl.add_static_constant("TYPE_STATE", umath::to_integral(pragma::ik::ControlType::State));
	classControl.def(luabind::tostring(luabind::self));
	classControl.def("GetTargetBone", static_cast<pragma::ik::Bone *(pragma::ik::IControl::*)()>(&pragma::ik::DragControl::GetTargetBone));
	classControl.def("GetType", &pragma::ik::IControl::GetControlType);
	modIk[classControl];

	auto classLinearMotorControl = luabind::class_<pragma::ik::ILinearMotorControl>("LinearMotorControl");
	classLinearMotorControl.def(luabind::tostring(luabind::self));
	classLinearMotorControl.def("GetTargetPosition", &pragma::ik::DragControl::GetTargetPosition);
	classLinearMotorControl.def("SetTargetPosition", &pragma::ik::DragControl::SetTargetPosition);
	classLinearMotorControl.def("GetOffset", &pragma::ik::DragControl::GetOffset);
	classLinearMotorControl.def("SetOffset", &pragma::ik::DragControl::SetOffset);
	modIk[classLinearMotorControl];

	auto classDragControl = luabind::class_<pragma::ik::DragControl, luabind::bases<pragma::ik::IControl, pragma::ik::ILinearMotorControl>>("DragControl");
	classDragControl.def(luabind::tostring(luabind::self));
	modIk[classDragControl];

	auto classAngularPlaneControl = luabind::class_<pragma::ik::AngularPlaneControl, pragma::ik::IControl>("AngularPlaneControl");
	classAngularPlaneControl.def(luabind::tostring(luabind::self));
	classAngularPlaneControl.def("SetPlaneNormal", &pragma::ik::AngularPlaneControl::SetPlaneNormal);
	classAngularPlaneControl.def("GetPlaneNormal", &pragma::ik::AngularPlaneControl::GetPlaneNormal);
	classAngularPlaneControl.def("SetBoneLocalAxis", &pragma::ik::AngularPlaneControl::SetBoneLocalAxis);
	classAngularPlaneControl.def("GetBoneLocalAxis", &pragma::ik::AngularPlaneControl::GetBoneLocalAxis);
	modIk[classAngularPlaneControl];

	auto classStateControl = luabind::class_<pragma::ik::StateControl, luabind::bases<pragma::ik::IControl, pragma::ik::ILinearMotorControl>>("StateControl");
	classStateControl.def(luabind::tostring(luabind::self));
	classStateControl.def("SetTargetOrientation", &pragma::ik::StateControl::SetTargetOrientation);
	classStateControl.def("GetTargetOrientation", &pragma::ik::StateControl::GetTargetOrientation);
	modIk[classStateControl];

	auto classSolver = luabind::class_<pragma::ik::Solver>("Solver");
	classSolver.def(luabind::tostring(luabind::self));
	classSolver.def("Solve", &pragma::ik::Solver::Solve);
	classSolver.def("AddDragControl", &pragma::ik::Solver::AddDragControl);
	classSolver.def("RemoveControl", &pragma::ik::Solver::RemoveControl);
	classSolver.def("AddAngularPlaneControl", &pragma::ik::Solver::AddAngularPlaneControl);
	classSolver.def("AddStateControl", &pragma::ik::Solver::AddStateControl);
	classSolver.def("AddBallSocketJoint", &pragma::ik::Solver::AddBallSocketJoint);
	classSolver.def("AddAngularJoint", &pragma::ik::Solver::AddAngularJoint);
	classSolver.def("AddPointOnLineJoint", &pragma::ik::Solver::AddPointOnLineJoint);
	classSolver.def("AddRevoluteJoint", &pragma::ik::Solver::AddRevoluteJoint);
	classSolver.def("AddDistanceJoint", &pragma::ik::Solver::AddDistanceJoint);
	classSolver.def("AddSwingLimit", &pragma::ik::Solver::AddSwingLimit);
	classSolver.def("AddEllipseSwingLimit", &pragma::ik::Solver::AddEllipseSwingLimit);
	classSolver.def("AddLinearAxisLimit", &pragma::ik::Solver::AddLinearAxisLimit);
	classSolver.def("AddTwistJoint", &pragma::ik::Solver::AddTwistJoint);
	classSolver.def("AddTwistLimit", &pragma::ik::Solver::AddTwistLimit);
	classSolver.def("AddSwivelHingeJoint", &pragma::ik::Solver::AddSwivelHingeJoint);
	classSolver.def(
	  "AddBone", +[](pragma::ik::Solver &solver, const std::string &name, const Vector3 &pos, const Quat &rot, float radius, float length) -> std::pair<pragma::ik::Bone *, pragma::ik::BoneId> {
		  pragma::ik::BoneId boneId;
		  auto &ikBone = solver.AddBone(pos, rot, radius, length, &boneId);
		  ikBone.SetName(name);
		  return {&ikBone, boneId};
	  });

	classSolver.def("GetControlCount", &pragma::ik::Solver::GetControlCount);
	classSolver.def("GetBoneCount", &pragma::ik::Solver::GetBoneCount);
	classSolver.def("GetJointCount", &pragma::ik::Solver::GetJointCount);
	classSolver.def("DebugPrint", &pragma::ik::debug_print);
	classSolver.def("GetControl", static_cast<pragma::ik::IControl *(pragma::ik::Solver::*)(size_t)>(&pragma::ik::Solver::GetControl));
	classSolver.def("GetBone", static_cast<pragma::ik::Bone *(pragma::ik::Solver::*)(pragma::ik::BoneId)>(&pragma::ik::Solver::GetBone));
	classSolver.def(
	  "GetJoint", +[](lua_State *l, pragma::ik::Solver &solver, size_t index) -> Lua::type<pragma::ik::IJoint> {
		  auto *joint = solver.GetJoint(index);
		  if(!joint)
			  return Lua::nil;
		  return joint_to_lua_object(l, *joint);
	  });
	modIk[classSolver];
	pragma::lua::define_custom_constructor<pragma::ik::Solver, [](uint32_t controlIterationCount, uint32_t fixerIterationCount) -> std::shared_ptr<pragma::ik::Solver> { return std::make_shared<pragma::ik::Solver>(controlIterationCount, fixerIterationCount); }, uint32_t, uint32_t>(
	  lua.GetState());
	pragma::lua::define_custom_constructor<pragma::ik::Solver, [](uint32_t controlIterationCount) -> std::shared_ptr<pragma::ik::Solver> { return std::make_shared<pragma::ik::Solver>(controlIterationCount); }, uint32_t>(lua.GetState());
	pragma::lua::define_custom_constructor<pragma::ik::Solver, []() -> std::shared_ptr<pragma::ik::Solver> { return std::make_shared<pragma::ik::Solver>(); }>(lua.GetState());
}