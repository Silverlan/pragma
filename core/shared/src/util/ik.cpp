/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/util/ik.hpp"
#include <sharedutils/util_pragma.hpp>

#include <bepuik/IKSolver.hpp>
#include <bepuik/joint/IKJoint.hpp>
#include <bepuik/joint/IKDistanceJoint.hpp>
#include <bepuik/limit/IKTwistLimit.hpp>
#include <bepuik/joint/IKTwistJoint.hpp>
#include <bepuik/joint/IKSwivelHingeJoint.hpp>
#include <bepuik/joint/IKBallSocketJoint.hpp>
#include <bepuik/joint/IKAngularJoint.hpp>
#include <bepuik/joint/IKRevoluteJoint.hpp>
#include <bepuik/limit/IKSwingLimit.hpp>
#include <bepuik/limit/IKEllipseSwingLimit.hpp>
#include <bepuik/joint/IKPointOnLineJoint.hpp>
#include <bepuik/limit/IKLinearAxisLimit.hpp>
#include <bepuik/control/DragControl.hpp>
#include <bepuik/control/AngularPlaneControl.hpp>
#include <bepuik/control/StateControl.hpp>
#include <bepuik/SingleBoneLinearMotor.hpp>
#include <bepuik/SingleBoneAngularMotor.hpp>
#include <bepuik/SingleBoneAngularPlaneConstraint.hpp>

//#define SWITCH_HANDEDNESS
// Bepuik does not work properly with large scales (limits are largely ineffective), so we need to scale accordingly
static auto to_bepu_scale = static_cast<float>(::util::pragma::units_to_metres(1.f));
static auto to_pragma_scale = static_cast<float>(::util::pragma::metres_to_units(1.f));
static BEPUik::Vector3 operator*(const BEPUik::Vector3 &v, float f) { return BEPUik::vector3::Multiply(const_cast<BEPUik::Vector3 &>(v), f); }
static BEPUik::Vector3 to_bepu_vector3(const Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return BEPUik::Vector3 {v.z, v.y, v.x} * to_bepu_scale;
#else
	return BEPUik::Vector3 {v.x, v.y, v.z} * to_bepu_scale;
#endif
}
static Vector3 from_bepu_vector3(const BEPUik::Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return Vector3 {v.Z, v.Y, v.X} * to_pragma_scale;
#else
	return Vector3 {v.x, v.y, v.z} * to_pragma_scale;
#endif
}
static BEPUik::Vector3 to_bepu_axis(const Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return BEPUik::Vector3 {v.z, v.y, v.x};
#else
	return BEPUik::Vector3 {v.x, v.y, v.z};
#endif
}
static Vector3 from_bepu_axis(const BEPUik::Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return Vector3 {v.Z, v.Y, v.X};
#else
	return Vector3 {v.x, v.y, v.z};
#endif
}
static BEPUik::Quaternion to_bepu_quaternion(const Quat &r)
{
#ifdef SWITCH_HANDEDNESS
	return BEPUik::Quaternion {-r.w, r.z, r.y, r.x};
#else
	return BEPUik::quaternion::Create(r.x, r.y, r.z, r.w);
#endif
}
static Quat from_bepu_quaternion(const BEPUik::Quaternion &r)
{
#ifdef SWITCH_HANDEDNESS
	return Quat {-r.W, r.Z, r.Y, r.X};
#else
	return Quat {r.w, r.x, r.y, r.z};
#endif
}
static float to_bepu_length(float l) { return l * to_bepu_scale; }
static float from_bepu_length(float l) { return l * to_pragma_scale; }

pragma::ik::Bone::Bone(const Vector3 &pos, const Quat &rot, float radius, float length, float mass) : m_origPose {pos,rot}
{
	m_bone = std::make_unique<BEPUik::Bone>();
	m_bone->Position = to_bepu_vector3(pos);
	m_bone->Orientation = to_bepu_quaternion(rot);
	m_bone->SetRadius(to_bepu_length(radius));
	m_bone->SetHeight(to_bepu_length(length));
	m_bone->SetInertiaTensorScaling(2.5f); // An arbitrary scaling factor is applied to the inertia tensor. This tends to improve stability.
	m_bone->SetMass(mass);
}
pragma::ik::Bone::~Bone() {}
bool pragma::ik::Bone::operator==(const Bone &other) const { return &other == this; }
const umath::Transform &pragma::ik::Bone::GetOriginalPose() const { return m_origPose; }
Vector3 pragma::ik::Bone::GetPos() const { return from_bepu_vector3(m_bone->Position); }
Quat pragma::ik::Bone::GetRot() const { return from_bepu_quaternion(m_bone->Orientation); }
void pragma::ik::Bone::SetPos(const Vector3 &pos) const { m_bone->Position = to_bepu_vector3(pos); }
void pragma::ik::Bone::SetRot(const Quat &rot) const { m_bone->Orientation = to_bepu_quaternion(rot); }
void pragma::ik::Bone::SetPinned(bool pinned) { m_bone->Pinned = pinned; }
bool pragma::ik::Bone::IsPinned() { return m_bone->Pinned; }
void pragma::ik::Bone::SetName(const std::string &name) { m_name = name; }
const std::string &pragma::ik::Bone::GetName() const { return m_name; }
float pragma::ik::Bone::GetRadius() const { return from_bepu_length(m_bone->GetRadius()); }
float pragma::ik::Bone::GetLength() const { return from_bepu_length(m_bone->GetHeight()); }
float pragma::ik::Bone::GetMass() const { return m_bone->GetMass(); }

BEPUik::Bone *pragma::ik::Bone::operator*() { return m_bone.get(); }
BEPUik::Bone *pragma::ik::Bone::operator->() { return m_bone.get(); }

pragma::ik::IControl::~IControl() {}
BEPUik::Control *pragma::ik::IControl::operator*() { return m_control.get(); }

pragma::ik::IControl::IControl() {}

void pragma::ik::IControl::SetTargetBone(Bone &bone)
{
	m_control->SetTargetBone(*bone);
	m_bone = &bone;
}
pragma::ik::Bone *pragma::ik::IControl::GetTargetBone() { return m_bone; }
const pragma::ik::Bone *pragma::ik::IControl::GetTargetBone() const { return const_cast<IControl *>(this)->GetTargetBone(); }

const BEPUik::SingleBoneLinearMotor &pragma::ik::ILinearMotorControl::GetLinearMotor() const { return const_cast<ILinearMotorControl *>(this)->GetLinearMotor(); }
void pragma::ik::ILinearMotorControl::SetTargetPosition(const Vector3 &pos) { GetLinearMotor().TargetPosition = to_bepu_vector3(pos); }
Vector3 pragma::ik::ILinearMotorControl::GetTargetPosition() const { return from_bepu_vector3(GetLinearMotor().TargetPosition); }

void pragma::ik::ILinearMotorControl::SetOffset(const Vector3 &offset) { GetLinearMotor().SetOffset(to_bepu_vector3(offset)); }
Vector3 pragma::ik::ILinearMotorControl::GetOffset() const { return from_bepu_vector3(GetLinearMotor().GetOffset()); }

pragma::ik::DragControl::DragControl(Bone &bone)
{
	auto ctrl = std::make_unique<BEPUik::DragControl>();
	m_control = std::move(ctrl);
	SetTargetBone(bone);
}
pragma::ik::DragControl::~DragControl() {}
BEPUik::SingleBoneLinearMotor &pragma::ik::DragControl::GetLinearMotor() { return *static_cast<BEPUik::DragControl *>(m_control.get())->GetLinearMotor(); }

pragma::ik::AngularPlaneControl::AngularPlaneControl(Bone &bone)
{
	auto ctrl = std::make_unique<BEPUik::AngularPlaneControl>();
	m_control = std::move(ctrl);
	SetTargetBone(bone);
}
pragma::ik::AngularPlaneControl::~AngularPlaneControl() {}
void pragma::ik::AngularPlaneControl::SetPlaneNormal(const Vector3 &n) { GetAngularMotor().PlaneNormal = to_bepu_axis(n); }
Vector3 pragma::ik::AngularPlaneControl::GetPlaneNormal() const { return from_bepu_axis(GetAngularMotor().PlaneNormal); }
void pragma::ik::AngularPlaneControl::SetBoneLocalAxis(const Vector3 &n) { GetAngularMotor().BoneLocalAxis = to_bepu_axis(n); }
Vector3 pragma::ik::AngularPlaneControl::GetBoneLocalAxis() const { return from_bepu_axis(GetAngularMotor().BoneLocalAxis); }

BEPUik::SingleBoneAngularPlaneConstraint &pragma::ik::AngularPlaneControl::GetAngularMotor() { return *static_cast<BEPUik::AngularPlaneControl *>(m_control.get())->GetAngularMotor(); }
const BEPUik::SingleBoneAngularPlaneConstraint &pragma::ik::AngularPlaneControl::GetAngularMotor() const { return const_cast<AngularPlaneControl *>(this)->GetAngularMotor(); }

pragma::ik::StateControl::StateControl(Bone &bone)
{
	auto ctrl = std::make_unique<BEPUik::StateControl>();
	m_control = std::move(ctrl);
	SetTargetBone(bone);
}
pragma::ik::StateControl::~StateControl() {}
void pragma::ik::StateControl::SetTargetOrientation(const Quat &rot) { static_cast<BEPUik::StateControl *>(m_control.get())->GetAngularMotor()->TargetOrientation = to_bepu_quaternion(rot); }
Quat pragma::ik::StateControl::GetTargetOrientation() const { return from_bepu_quaternion(static_cast<BEPUik::StateControl *>(m_control.get())->GetAngularMotor()->TargetOrientation); }
BEPUik::SingleBoneLinearMotor &pragma::ik::StateControl::GetLinearMotor() { return *static_cast<BEPUik::StateControl *>(m_control.get())->GetLinearMotor(); }

void pragma::ik::IJoint::SetJoint(std::unique_ptr<BEPUik::IKJoint> joint, Bone &connectionA, Bone &connectionB)
{
	m_joint = std::move(joint);
	m_connectionA = &connectionA;
	m_connectionB = &connectionB;
}

pragma::ik::IJoint::IJoint(JointType type) : m_jointType {type} {}
pragma::ik::IJoint::~IJoint() {}

void pragma::ik::IJoint::SetRigidity(float rigidity) { m_joint->SetRigidity(rigidity); }
float pragma::ik::IJoint::GetRigidity() { return m_joint->GetRigidity(); }

pragma::ik::Bone &pragma::ik::IJoint::GetConnectionA() { return *m_connectionA; }
const pragma::ik::Bone &pragma::ik::IJoint::GetConnectionA() const { return const_cast<pragma::ik::IJoint *>(this)->GetConnectionA(); }
pragma::ik::Bone &pragma::ik::IJoint::GetConnectionB() { return *m_connectionB; }
const pragma::ik::Bone &pragma::ik::IJoint::GetConnectionB() const { return const_cast<pragma::ik::IJoint *>(this)->GetConnectionB(); }

BEPUik::IKJoint *pragma::ik::IJoint::operator*() { return m_joint.get(); }
BEPUik::IKJoint *pragma::ik::IJoint::operator->() { return m_joint.get(); }

pragma::ik::IJoint::IJoint() {}

pragma::ik::DistanceJoint::DistanceJoint(Bone &bone0, Bone &bone1) : IJoint {JointType::DistanceJoint}
{
	auto joint = std::make_unique<BEPUik::IKDistanceJoint>(**bone0, **bone1, bone0->Position, bone1->Position);
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::DistanceJoint::~DistanceJoint() {}

pragma::ik::PointOnLineJoint::PointOnLineJoint(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB) : IJoint {JointType::PointOnLineJoint}, m_lineAnchor {lineAnchor}, m_lineDirection {lineDirection}, m_anchorB {anchorB}
{
	auto joint = std::make_unique<BEPUik::IKPointOnLineJoint>(**bone0, **bone1, to_bepu_vector3(lineAnchor), to_bepu_axis(lineDirection), to_bepu_vector3(anchorB));
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::PointOnLineJoint::~PointOnLineJoint() {}
const Vector3 &pragma::ik::PointOnLineJoint::GetLineAnchor() const { return m_lineAnchor; }
const Vector3 &pragma::ik::PointOnLineJoint::GetLineDirection() const {return m_lineDirection;}
const Vector3 &pragma::ik::PointOnLineJoint::GetAnchorB() const { return m_anchorB; }

pragma::ik::BallSocketJoint::BallSocketJoint(Bone &bone0, Bone &bone1, const Vector3 &anchor) : IJoint {JointType::BallSocketJoint}, m_anchor {anchor}
{
	auto joint = std::make_unique<BEPUik::IKBallSocketJoint>(**bone0, **bone1, to_bepu_vector3(anchor));
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::BallSocketJoint::~BallSocketJoint() {}
const Vector3 &pragma::ik::BallSocketJoint::GetAnchor() const { return m_anchor; }
Vector3 pragma::ik::BallSocketJoint::GetOffsetA() const { return from_bepu_vector3(static_cast<BEPUik::IKBallSocketJoint *>(m_joint.get())->GetOffsetA()); }
Vector3 pragma::ik::BallSocketJoint::GetOffsetB() const { return from_bepu_vector3(static_cast<BEPUik::IKBallSocketJoint *>(m_joint.get())->GetOffsetB()); }

pragma::ik::AngularJoint::AngularJoint(Bone &bone0, Bone &bone1) : IJoint {JointType::AngularJoint}
{
	auto joint = std::make_unique<BEPUik::IKAngularJoint>(**bone0, **bone1);
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::AngularJoint::~AngularJoint() {}

pragma::ik::RevoluteJoint::RevoluteJoint(Bone &bone0, Bone &bone1, const Vector3 &freeAxis) : IJoint {JointType::RevoluteJoint}, m_freeAxis {freeAxis}
{
	auto joint = std::make_unique<BEPUik::IKRevoluteJoint>(**bone0, **bone1, to_bepu_axis(freeAxis));
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::RevoluteJoint::~RevoluteJoint() {}
const Vector3 &pragma::ik::RevoluteJoint::GetFreeAxis() const { return m_freeAxis; }

pragma::ik::TwistJoint::TwistJoint(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB) : IJoint {JointType::TwistJoint}, m_axisA {axisA}, m_axisB {axisB}
{
	auto joint = std::make_unique<BEPUik::IKTwistJoint>(**bone0, **bone1, to_bepu_axis(axisA), to_bepu_axis(axisB));
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::TwistJoint::~TwistJoint() {}
const Vector3 &pragma::ik::TwistJoint::GetAxisA() const { return m_axisA; }
const Vector3 &pragma::ik::TwistJoint::GetAxisB() const { return m_axisB; }
void pragma::ik::TwistJoint::SetAxisA(const Vector3 &axisA) {
	static_cast<BEPUik::IKTwistJoint *>(m_joint.get())->SetAxisA(to_bepu_axis(axisA));
	static_cast<BEPUik::IKTwistJoint *>(m_joint.get())->ComputeMeasurementAxes();
}
void pragma::ik::TwistJoint::SetAxisB(const Vector3 &axisB)
{
	static_cast<BEPUik::IKTwistJoint *>(m_joint.get())->SetAxisB(to_bepu_axis(axisB));
	static_cast<BEPUik::IKTwistJoint *>(m_joint.get())->ComputeMeasurementAxes();
}

pragma::ik::SwingLimit::SwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Radian maxAngle) : IJoint {JointType::SwingLimit}, m_maxAngle {maxAngle}
{
	auto joint = std::make_unique<BEPUik::IKSwingLimit>(**bone0, **bone1, to_bepu_axis(axisA), to_bepu_axis(axisB), maxAngle);
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::SwingLimit::~SwingLimit() {}
umath::Radian pragma::ik::SwingLimit::GetMaxAngle() const { return m_maxAngle; }
Vector3 pragma::ik::SwingLimit::GetAxisA() const { return from_bepu_axis(static_cast<BEPUik::IKSwingLimit *>(m_joint.get())->GetAxisA()); }
Vector3 pragma::ik::SwingLimit::GetAxisB() const { return from_bepu_axis(static_cast<BEPUik::IKSwingLimit *>(m_joint.get())->GetAxisB()); }

void pragma::ik::SwingLimit::SetAxisA(const Vector3 &axisA) { static_cast<BEPUik::IKSwingLimit *>(m_joint.get())->SetAxisA(to_bepu_axis(axisA)); }
void pragma::ik::SwingLimit::SetAxisB(const Vector3 &axisB) { static_cast<BEPUik::IKSwingLimit *>(m_joint.get())->SetAxisB(to_bepu_axis(axisB)); }
void pragma::ik::SwingLimit::SetMaxAngle(umath::Radian maxAngle) { static_cast<BEPUik::IKSwingLimit *>(m_joint.get())->SetMaximumAngle(maxAngle); }

pragma::ik::EllipseSwingLimit::EllipseSwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, const Vector3 &xAxis, umath::Radian maxAngleX, umath::Radian maxAngleY)
    : IJoint {JointType::EllipseSwingLimit}, m_maxAngleX {maxAngleX}, m_maxAngleY {maxAngleY}, m_axisA {axisA}, m_axisB {axisB}, m_xAxis {xAxis}
{
	auto joint = std::make_unique<BEPUik::IKEllipseSwingLimit>(**bone0, **bone1, to_bepu_axis(axisA), to_bepu_axis(axisB), to_bepu_axis(xAxis), maxAngleX, maxAngleY);
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::EllipseSwingLimit::~EllipseSwingLimit() {}
const Vector3 &pragma::ik::EllipseSwingLimit::GetAxisA() const { return m_axisA; }
const Vector3 &pragma::ik::EllipseSwingLimit::GetAxisB() const { return m_axisB; }
const Vector3 &pragma::ik::EllipseSwingLimit::GetXAxis() const { return m_xAxis; }
umath::Radian pragma::ik::EllipseSwingLimit::GetMaxAngleX() const { return m_maxAngleX; }
umath::Radian pragma::ik::EllipseSwingLimit::GetMaxAngleY() const { return m_maxAngleY; }

pragma::ik::LinearAxisLimit::LinearAxisLimit(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB, float minimumDistance, float maximumDistance)
    : IJoint {JointType::LinearAxisLimit}, m_lineAnchor {lineAnchor}, m_lineDirection {lineDirection}, m_anchorB {anchorB}, m_minimumDistance {minimumDistance}, m_maximumDistance {maximumDistance}
{
	auto joint = std::make_unique<BEPUik::IKLinearAxisLimit>(**bone0, **bone1, to_bepu_vector3(lineAnchor), to_bepu_axis(lineDirection), to_bepu_vector3(anchorB), to_bepu_length(minimumDistance), to_bepu_length(maximumDistance));
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::LinearAxisLimit::~LinearAxisLimit() {}
const Vector3 &pragma::ik::LinearAxisLimit::GetLineAnchor() const { return m_lineAnchor; }
const Vector3 &pragma::ik::LinearAxisLimit::GetLineDirection() const { return m_lineDirection; }
const Vector3 &pragma::ik::LinearAxisLimit::GetAnchorB() const { return m_anchorB; }
float pragma::ik::LinearAxisLimit::GetMinimumDistance() const { return m_minimumDistance; }
float pragma::ik::LinearAxisLimit::GetMaximumDistance() const { return m_maximumDistance; }

pragma::ik::TwistLimit::TwistLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, umath::Radian maxAngle) : IJoint {JointType::TwistLimit}, m_maxAngle {maxAngle}, m_axisA {axisA}, m_axisB {axisB}
{
	auto joint = std::make_unique<BEPUik::IKTwistLimit>(**bone0, **bone1, to_bepu_axis(axisA), to_bepu_axis(axisB), maxAngle);
	joint->ComputeMeasurementAxes();
	SetJoint(std::move(joint), bone0, bone1);
}
void pragma::ik::TwistLimit::SetMeasurementAxisA(const Vector3 &axis) { static_cast<BEPUik::IKTwistLimit *>(m_joint.get())->SetMeasurementAxisA(to_bepu_axis(axis)); }
Vector3 pragma::ik::TwistLimit::GetMeasurementAxisA() { return from_bepu_axis(static_cast<BEPUik::IKTwistLimit *>(m_joint.get())->GetMeasurementAxisA()); }
void pragma::ik::TwistLimit::SetMeasurementAxisB(const Vector3 &axis) { static_cast<BEPUik::IKTwistLimit *>(m_joint.get())->SetMeasurementAxisB(to_bepu_axis(axis)); }
Vector3 pragma::ik::TwistLimit::GetMeasurementAxisB() { return from_bepu_axis(static_cast<BEPUik::IKTwistLimit *>(m_joint.get())->GetMeasurementAxisB()); }
umath::Radian pragma::ik::TwistLimit::GetMaxAngle() const { return m_maxAngle; }
const Vector3 &pragma::ik::TwistLimit::GetAxisA() const { return m_axisA; }
const Vector3 &pragma::ik::TwistLimit::GetAxisB() const { return m_axisB; }
pragma::ik::TwistLimit::~TwistLimit() {}

pragma::ik::SwivelHingeJoint::SwivelHingeJoint(Bone &bone0, Bone &bone1, const Vector3 &worldHingeAxis, const Vector3 &worldTwistAxis) : IJoint {JointType::SwivelHingeJoint}, m_worldHingeAxis {worldHingeAxis}, m_worldTwistAxis {worldTwistAxis}
{
	auto joint = std::make_unique<BEPUik::IKSwivelHingeJoint>(**bone0, **bone1, to_bepu_axis(worldHingeAxis), to_bepu_axis(worldTwistAxis));
	SetJoint(std::move(joint), bone0, bone1);
}
pragma::ik::SwivelHingeJoint::~SwivelHingeJoint() {}
const Vector3 &pragma::ik::SwivelHingeJoint::GetWorldHingeAxis() const { return m_worldHingeAxis; }
const Vector3 &pragma::ik::SwivelHingeJoint::GetWorldTwistAxis() const { return m_worldTwistAxis; }

pragma::ik::Solver::Solver(uint32_t controlIterationCount, uint32_t fixerIterationCount)
{
	m_solver = std::make_unique<BEPUik::IKSolver>();
	m_solver->activeSet.UseAutomass = true;
	m_solver->AutoscaleControlImpulses = true;
	m_solver->AutoscaleControlMaximumForce = std::numeric_limits<float>::max();
	m_solver->SetTimeStepDuration(0.1f);
	m_solver->ControlIterationCount = controlIterationCount;
	m_solver->FixerIterationCount = fixerIterationCount;
	m_solver->VelocitySubiterationCount = 3;
}
pragma::ik::Solver::~Solver() {}
void pragma::ik::Solver::Solve() { m_solver->Solve(m_bepuControls); }
pragma::ik::DragControl &pragma::ik::Solver::AddDragControl(Bone &bone)
{
	auto ctrl = std::make_shared<DragControl>(bone);
	m_controls.push_back(ctrl);
	m_bepuControls.push_back(**ctrl);
	return *ctrl;
}
void pragma::ik::Solver::RemoveControl(const IControl &ctrl)
{
	auto it = std::find_if(m_controls.begin(), m_controls.end(), [&ctrl](const std::shared_ptr<IControl> &other) { return &ctrl == other.get(); });
	if(it == m_controls.end())
		return;
	auto idx = it - m_controls.begin();
	m_controls.erase(it);
	m_bepuControls.erase(m_bepuControls.begin() + idx);
}
pragma::ik::AngularPlaneControl &pragma::ik::Solver::AddAngularPlaneControl(Bone &bone)
{
	auto ctrl = std::make_shared<AngularPlaneControl>(bone);
	m_controls.push_back(ctrl);
	m_bepuControls.push_back(**ctrl);
	return *ctrl;
}
std::shared_ptr<pragma::ik::IControl> pragma::ik::Solver::FindControlPtr(Bone &bone)
{
	auto *bepuIkBone = *bone;
	for(auto &ctrl : m_controls) {
		auto *bepuCtrl = **ctrl;
		if(bepuCtrl->GetTargetBone() == bepuIkBone)
			return ctrl;
	}
	return nullptr;
}
pragma::ik::IControl *pragma::ik::Solver::FindControl(Bone &bone)
{
	auto *bepuIkBone = *bone;
	for(auto &ctrl : m_controls) {
		auto *bepuCtrl = **ctrl;
		if(bepuCtrl->GetTargetBone() == bepuIkBone)
			return ctrl.get();
	}
	return nullptr;
}
pragma::ik::StateControl &pragma::ik::Solver::AddStateControl(Bone &bone)
{
	auto ctrl = std::make_shared<StateControl>(bone);
	m_controls.push_back(ctrl);
	m_bepuControls.push_back(**ctrl);
	return *ctrl;
}
pragma::ik::DistanceJoint &pragma::ik::Solver::AddDistanceJoint(Bone &bone0, Bone &bone1)
{
	auto joint = std::make_shared<DistanceJoint>(bone0, bone1);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::BallSocketJoint &pragma::ik::Solver::AddBallSocketJoint(Bone &bone0, Bone &bone1, const Vector3 &anchor)
{
	auto joint = std::make_shared<BallSocketJoint>(bone0, bone1, anchor);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::AngularJoint &pragma::ik::Solver::AddAngularJoint(Bone &bone0, Bone &bone1)
{
	auto joint = std::make_shared<AngularJoint>(bone0, bone1);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::PointOnLineJoint &pragma::ik::Solver::AddPointOnLineJoint(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB)
{
	auto joint = std::make_shared<PointOnLineJoint>(bone0, bone1, lineAnchor, lineDirection, anchorB);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::RevoluteJoint &pragma::ik::Solver::AddRevoluteJoint(Bone &bone0, Bone &bone1, const Vector3 &freeAxis)
{
	auto joint = std::make_shared<RevoluteJoint>(bone0, bone1, freeAxis);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::SwingLimit &pragma::ik::Solver::AddSwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, float maxAngle)
{
	auto joint = std::make_shared<SwingLimit>(bone0, bone1, axisA, axisB, maxAngle);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::EllipseSwingLimit &pragma::ik::Solver::AddEllipseSwingLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, const Vector3 &xAxis, float maxAngleX, float maxAngleY)
{
	auto joint = std::make_shared<EllipseSwingLimit>(bone0, bone1, axisA, axisB, xAxis, maxAngleX, maxAngleY);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::LinearAxisLimit &pragma::ik::Solver::AddLinearAxisLimit(Bone &bone0, Bone &bone1, const Vector3 &lineAnchor, const Vector3 &lineDirection, const Vector3 &anchorB, float minimumDistance, float maximumDistance)
{
	auto joint = std::make_shared<LinearAxisLimit>(bone0, bone1, lineAnchor, lineDirection, anchorB, minimumDistance, maximumDistance);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::TwistJoint &pragma::ik::Solver::AddTwistJoint(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB)
{
	auto joint = std::make_shared<TwistJoint>(bone0, bone1, axisA, axisB);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::TwistLimit &pragma::ik::Solver::AddTwistLimit(Bone &bone0, Bone &bone1, const Vector3 &axisA, const Vector3 &axisB, float maxAngle)
{
	auto joint = std::make_shared<TwistLimit>(bone0, bone1, axisA, axisB, maxAngle);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::SwivelHingeJoint &pragma::ik::Solver::AddSwivelHingeJoint(Bone &bone0, Bone &bone1, const Vector3 &worldHingeAxis, const Vector3 &worldTwistAxis)
{
	auto joint = std::make_shared<SwivelHingeJoint>(bone0, bone1, worldHingeAxis, worldTwistAxis);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::Bone &pragma::ik::Solver::AddBone(const Vector3 &pos, const Quat &rot, float radius, float length, BoneId *optOutBoneId)
{
	auto bone = std::make_shared<Bone>(pos, rot, radius, length, 1.f);
	m_bones.push_back(bone);
	m_bepuBones.push_back(**bone);
	if(optOutBoneId)
		*optOutBoneId = m_bones.size() - 1;
	return *bone;
}

size_t pragma::ik::Solver::GetControlCount() const { return m_controls.size(); }
size_t pragma::ik::Solver::GetBoneCount() const { return m_bones.size(); }
size_t pragma::ik::Solver::GetJointCount() const { return m_joints.size(); }

pragma::ik::IControl *pragma::ik::Solver::GetControl(size_t index) { return (index < m_controls.size()) ? m_controls[index].get() : nullptr; }
const pragma::ik::IControl *pragma::ik::Solver::GetControl(size_t index) const { return const_cast<Solver *>(this)->GetControl(index); }
pragma::ik::Bone *pragma::ik::Solver::GetBone(BoneId index) { return (index < m_bones.size()) ? m_bones[index].get() : nullptr; }
const pragma::ik::Bone *pragma::ik::Solver::GetBone(BoneId index) const { return const_cast<Solver *>(this)->GetBone(index); }
pragma::ik::IJoint *pragma::ik::Solver::GetJoint(size_t index) { return (index < m_joints.size()) ? m_joints[index].get() : nullptr; }
const pragma::ik::IJoint *pragma::ik::Solver::GetJoint(size_t index) const { return const_cast<Solver *>(this)->GetJoint(index); }

const std::vector<std::shared_ptr<pragma::ik::IControl>> &pragma::ik::Solver::GetControls() const { return m_controls; }
const std::vector<std::shared_ptr<pragma::ik::Bone>> &pragma::ik::Solver::GetBones() const { return m_bones; }
const std::vector<std::shared_ptr<pragma::ik::IJoint>> &pragma::ik::Solver::GetJoints() const { return m_joints; }

std::ostream &operator<<(std::ostream &out, const pragma::ik::Bone &bone)
{
	out << "Bone";
	out << "[Name:" << bone.GetName() << "]";
	out << "[Pos:" << bone.GetPos() << "]";
	auto ang = EulerAngles {bone.GetRot()};
	out << "[Ang:" << ang << "]";
	return out;
}
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

std::ostream &operator<<(std::ostream &out, const pragma::ik::IJoint &joint)
{
	out << magic_enum::enum_name(joint.GetJointType());
	out << "[ConA:" << joint.GetConnectionA().GetName() << "]";
	out << "[ConB:" << joint.GetConnectionB().GetName() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::BallSocketJoint &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[Anchor:" << joint.GetAnchor() << "][OffsetA:" << joint.GetOffsetA() << "][OffsetB:" << joint.GetOffsetB() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::AngularJoint &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::PointOnLineJoint &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[LineDir:" << joint.GetLineDirection() << "]";
	out << "[AnchorB : " << joint.GetAnchorB() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::RevoluteJoint &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[FreeAxis:" << joint.GetFreeAxis() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::SwingLimit &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[AxisA:" << joint.GetAxisA() << "]";
	out << "[AxisB:" << joint.GetAxisB() << "]";
	out << "[MaxAngle:" << umath::rad_to_deg(joint.GetMaxAngle()) << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::EllipseSwingLimit &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[AxisA:" << joint.GetAxisA() << "]";
	out << "[AxisB:" << joint.GetAxisB() << "]";
	out << "[XAxis:" << joint.GetXAxis() << "]";
	out << "[MaxAngleX:" << umath::rad_to_deg(joint.GetMaxAngleX()) << "]";
	out << "[MaxAngleY:" << umath::rad_to_deg(joint.GetMaxAngleY()) << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::LinearAxisLimit &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[LineAnchor:" << joint.GetLineAnchor() << "]";
	out << "[LineDir:" << joint.GetLineDirection() << "]";
	out << "[AnchorB:" << joint.GetAnchorB() << "]";
	out << "[MinDist:" << joint.GetMinimumDistance() << "]";
	out << "[MaxDist:" << joint.GetMaximumDistance() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::TwistJoint &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[AxisA:" << joint.GetAxisA() << "]";
	out << "[AxisB:" << joint.GetAxisB() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::TwistLimit &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[AxisA:" << joint.GetAxisA() << "]";
	out << "[AxisB:" << joint.GetAxisB() << "]";
	out << "[MaxAngle:" << umath::rad_to_deg(joint.GetMaxAngle()) << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::SwivelHingeJoint &joint)
{
	operator<<(out, static_cast<const pragma::ik::IJoint &>(joint));
	out << "[WorldHingeAxis:" << joint.GetWorldHingeAxis() << "]";
	out << "[WorldTwistAxis:" << joint.GetWorldTwistAxis() << "]";
	return out;
}

static std::ostream &print_control_bone(std::ostream &out, const pragma::ik::IControl &control)
{
	auto *bone = control.GetTargetBone();
	out << "[Bone:";
	if(bone)
		out << bone->GetName();
	else
		out << "NULL";
	out << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::IControl &control)
{
	out << "Control";
	return print_control_bone(out, control);
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::ILinearMotorControl &control)
{
	out << "[Offset:" << control.GetOffset() << "]";
	out << "[TargetPos:" << control.GetTargetPosition() << "]";
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::DragControl &control)
{
	out << "DragControl";
	operator<<(out, static_cast<const pragma::ik::ILinearMotorControl &>(control));
	return print_control_bone(out, control);
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::AngularPlaneControl &control)
{
	out << "AngularPlaneControl";
	out << "[BoneLocalAxis:" << control.GetBoneLocalAxis() << "]";
	out << "[PlaneNormal:" << control.GetPlaneNormal() << "]";
	operator<<(out, static_cast<const pragma::ik::IControl &>(control));
	return out;
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::StateControl &control)
{
	out << "StateControl";
	auto ang = EulerAngles {control.GetTargetOrientation()};
	out << "[TargetAng:" << ang << "]";
	operator<<(out, static_cast<const pragma::ik::ILinearMotorControl &>(control));
	return print_control_bone(out, control);
}
std::ostream &operator<<(std::ostream &out, const pragma::ik::Solver &solver)
{
	out << "IkSolver";
	out << "[Bones:" << solver.GetBoneCount() << "]";
	out << "[Controls:" << solver.GetControlCount() << "]";
	out << "[Joints:" << solver.GetJointCount() << "]";
	return out;
}

void pragma::ik::debug_print(const pragma::ik::Solver &solver)
{
	std::stringstream ss;
	ss << solver << "\n";
	ss << "Bones:\n";
	for(auto i = decltype(solver.GetBoneCount()) {0u}; i < solver.GetBoneCount(); ++i) {
		auto &bone = *solver.GetBone(i);
		ss << "\t" << bone << "\n";
	}
	ss << "Controls:\n";
	for(auto i = decltype(solver.GetControlCount()) {0u}; i < solver.GetControlCount(); ++i) {
		auto &control = *solver.GetControl(i);
		if(typeid(control) == typeid(pragma::ik::DragControl))
			ss << "\t" << static_cast<const pragma::ik::DragControl &>(control) << "\n";
		else if(typeid(control) == typeid(pragma::ik::AngularPlaneControl))
			ss << "\t" << static_cast<const pragma::ik::AngularPlaneControl &>(control) << "\n";
		else if(typeid(control) == typeid(pragma::ik::StateControl))
			ss << "\t" << static_cast<const pragma::ik::StateControl &>(control) << "\n";
		else
			ss << "\t" << control << "\n";
	}
	ss << "Joints:\n";
	for(auto i = decltype(solver.GetJointCount()) {0u}; i < solver.GetJointCount(); ++i) {
		auto &joint = *solver.GetJoint(i);
		auto type = joint.GetJointType();
		if(type == pragma::ik::JointType::DistanceJoint) {
			if(i > 0)
				ss << "\n";
			ss << "\t" << static_cast<const pragma::ik::DistanceJoint &>(joint) << "\n";
		}
		else if(type == pragma::ik::JointType::BallSocketJoint) {
			if(i > 0)
				ss << "\n";
			ss << "\t" << static_cast<const pragma::ik::BallSocketJoint &>(joint) << "\n";
		}
		else if(type == pragma::ik::JointType::AngularJoint) {
			if(i > 0)
				ss << "\n";
			ss << "\t" << static_cast<const pragma::ik::AngularJoint &>(joint) << "\n";
		}
		else if(type == pragma::ik::JointType::PointOnLineJoint) {
			if(i > 0)
				ss << "\n";
			ss << "\t" << static_cast<const pragma::ik::PointOnLineJoint &>(joint) << "\n";
		}
		else if(type == pragma::ik::JointType::RevoluteJoint) {
			if(i > 0)
				ss << "\n";
			ss << "\t" << static_cast<const pragma::ik::RevoluteJoint &>(joint) << "\n";
		}
		else if(type == pragma::ik::JointType::SwingLimit)
			ss << "\t" << static_cast<const pragma::ik::SwingLimit &>(joint) << "\n";
		else if(type == pragma::ik::JointType::EllipseSwingLimit)
			ss << "\t" << static_cast<const pragma::ik::EllipseSwingLimit &>(joint) << "\n";
		else if(type == pragma::ik::JointType::LinearAxisLimit)
			ss << "\t" << static_cast<const pragma::ik::LinearAxisLimit &>(joint) << "\n";
		else if(type == pragma::ik::JointType::TwistJoint) {
			if(i > 0)
				ss << "\n";
			ss << "\t" << static_cast<const pragma::ik::TwistJoint &>(joint) << "\n";
		}
		else if(type == pragma::ik::JointType::TwistLimit)
			ss << "\t" << static_cast<const pragma::ik::TwistLimit &>(joint) << "\n";
		else if(type == pragma::ik::JointType::SwivelHingeJoint) {
			if(i > 0)
				ss << "\n";
			ss << "\t" << static_cast<const pragma::ik::SwivelHingeJoint &>(joint) << "\n";
		}
		else
			ss << "\t" << joint << "\n";
		static_assert(umath::to_integral(pragma::ik::JointType::Count) == 11, "Update this implementation when joints are removed or added!");
	}
	Con::cout << ss.str() << Con::endl;
}

