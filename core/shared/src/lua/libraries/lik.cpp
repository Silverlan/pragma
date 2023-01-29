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
#include <luainterface.hpp>

namespace Lua::ik {
	void register_library(Lua::Interface &lua);
};

void Lua::ik::register_library(Lua::Interface &lua)
{
	auto *l = lua.GetState();

	const auto *libName = "ik";
	auto &modIk = lua.RegisterLibrary(libName);

	auto classBone = luabind::class_<pragma::ik::Bone>("Bone");
	classBone.def("GetPos", &pragma::ik::Bone::GetPos);
	classBone.def("SetPos", &pragma::ik::Bone::SetPos);
	classBone.def("GetRot", &pragma::ik::Bone::GetRot);
	classBone.def("SetRot", &pragma::ik::Bone::SetRot);
	classBone.def("SetPinned", &pragma::ik::Bone::SetPinned);
	classBone.def("IsPinned", &pragma::ik::Bone::IsPinned);
	modIk[classBone];

	auto classJoint = luabind::class_<pragma::ik::IJoint>("Joint");
	classJoint.def("SetRigidity", &pragma::ik::IJoint::SetRigidity);
	classJoint.def("GetRigidity", &pragma::ik::IJoint::GetRigidity);
	modIk[classJoint];

	auto classDistanceJoint = luabind::class_<pragma::ik::DistanceJoint, pragma::ik::IJoint>("DistanceJoint");
	modIk[classDistanceJoint];

	auto classBallSocketJoint = luabind::class_<pragma::ik::BallSocketJoint, pragma::ik::IJoint>("BallSocketJoint");
	modIk[classBallSocketJoint];

	auto classAngularJoint = luabind::class_<pragma::ik::AngularJoint, pragma::ik::IJoint>("AngularJoint");
	modIk[classAngularJoint];

	auto classPointOnLineJoint = luabind::class_<pragma::ik::PointOnLineJoint, pragma::ik::IJoint>("PointOnLineJoint");
	modIk[classPointOnLineJoint];

	auto classRevoluteJoint = luabind::class_<pragma::ik::RevoluteJoint, pragma::ik::IJoint>("RevoluteJoint");
	modIk[classRevoluteJoint];

	auto classSwingLimit = luabind::class_<pragma::ik::SwingLimit, pragma::ik::IJoint>("SwingLimit");
	modIk[classSwingLimit];

	auto classEllipseSwingLimit = luabind::class_<pragma::ik::EllipseSwingLimit, pragma::ik::IJoint>("EllipseSwingLimit");
	modIk[classEllipseSwingLimit];

	auto classLinearAxisLimit = luabind::class_<pragma::ik::LinearAxisLimit, pragma::ik::IJoint>("LinearAxisLimit");
	modIk[classLinearAxisLimit];

	auto classTwistJoint = luabind::class_<pragma::ik::TwistJoint, pragma::ik::IJoint>("TwistJoint");
	modIk[classTwistJoint];

	auto classTwistLimit = luabind::class_<pragma::ik::TwistLimit, pragma::ik::IJoint>("TwistLimit");
	classTwistLimit.def("SetMeasurementAxisA", &pragma::ik::TwistLimit::SetMeasurementAxisA);
	classTwistLimit.def("GetMeasurementAxisA", &pragma::ik::TwistLimit::GetMeasurementAxisA);
	classTwistLimit.def("SetMeasurementAxisB", &pragma::ik::TwistLimit::SetMeasurementAxisB);
	classTwistLimit.def("GetMeasurementAxisB", &pragma::ik::TwistLimit::GetMeasurementAxisB);
	modIk[classTwistLimit];

	auto classSwivelHingeJoint = luabind::class_<pragma::ik::SwivelHingeJoint, pragma::ik::IJoint>("SwivelHingeJoint");
	modIk[classSwivelHingeJoint];

	auto classControl = luabind::class_<pragma::ik::IControl>("Control");
	modIk[classControl];

	auto classLinearMotorControl = luabind::class_<pragma::ik::ILinearMotorControl>("LinearMotorControl");
	classLinearMotorControl.def("GetTargetPosition", &pragma::ik::DragControl::GetTargetPosition);
	classLinearMotorControl.def("SetTargetPosition", &pragma::ik::DragControl::SetTargetPosition);
	modIk[classLinearMotorControl];

	auto classDragControl = luabind::class_<pragma::ik::DragControl, luabind::bases<pragma::ik::IControl, pragma::ik::ILinearMotorControl>>("DragControl");
	modIk[classDragControl];

	auto classAngularPlaneControl = luabind::class_<pragma::ik::AngularPlaneControl, pragma::ik::IControl>("AngularPlaneControl");
	classAngularPlaneControl.def("SetPlaneNormal", &pragma::ik::AngularPlaneControl::SetPlaneNormal);
	classAngularPlaneControl.def("GetPlaneNormal", &pragma::ik::AngularPlaneControl::GetPlaneNormal);
	classAngularPlaneControl.def("SetBoneLocalAxis", &pragma::ik::AngularPlaneControl::SetBoneLocalAxis);
	classAngularPlaneControl.def("GetBoneLocalAxis", &pragma::ik::AngularPlaneControl::GetBoneLocalAxis);
	modIk[classAngularPlaneControl];

	auto classStateControl = luabind::class_<pragma::ik::StateControl, luabind::bases<pragma::ik::IControl, pragma::ik::ILinearMotorControl>>("StateControl");
	classStateControl.def("SetTargetOrientation", &pragma::ik::StateControl::SetTargetOrientation);
	classStateControl.def("GetTargetOrientation", &pragma::ik::StateControl::GetTargetOrientation);
	modIk[classStateControl];

	auto classSolver = luabind::class_<pragma::ik::Solver>("Solver");
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
	classSolver.def("AddBone", &pragma::ik::Solver::AddBone);

	classSolver.def("GetControlCount", &pragma::ik::Solver::GetControlCount);
	classSolver.def("GetBoneCount", &pragma::ik::Solver::GetBoneCount);
	classSolver.def("GetJointCount", &pragma::ik::Solver::GetJointCount);
	classSolver.def("GetControl", &pragma::ik::Solver::GetControl);
	classSolver.def("GetBone", &pragma::ik::Solver::GetBone);
	classSolver.def("GetJoint", &pragma::ik::Solver::GetJoint);
	modIk[classSolver];
	pragma::lua::define_custom_constructor<pragma::ik::Solver, [](uint32_t controlIterationCount, uint32_t fixerIterationCount) -> std::shared_ptr<pragma::ik::Solver> { return std::make_shared<pragma::ik::Solver>(controlIterationCount, fixerIterationCount); }, uint32_t, uint32_t>(
	  lua.GetState());
	pragma::lua::define_custom_constructor<pragma::ik::Solver, [](uint32_t controlIterationCount) -> std::shared_ptr<pragma::ik::Solver> { return std::make_shared<pragma::ik::Solver>(controlIterationCount); }, uint32_t>(lua.GetState());
	pragma::lua::define_custom_constructor<pragma::ik::Solver, []() -> std::shared_ptr<pragma::ik::Solver> { return std::make_shared<pragma::ik::Solver>(); }>(lua.GetState());
}
