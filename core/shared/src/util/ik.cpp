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
static auto to_bepu_scale = static_cast<float>(::util::pragma::units_to_metres(1.f));
static auto to_pragma_scale = static_cast<float>(::util::pragma::metres_to_units(1.f));
static BEPUik::Vector3 operator*(const BEPUik::Vector3 &v,float f)
{
	return BEPUik::vector3::Multiply(const_cast<BEPUik::Vector3&>(v),f);
}
static BEPUik::Vector3 to_bepu_vector3(const Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return BEPUik::Vector3{v.z,v.y,v.x} *to_bepu_scale;
#else
	return BEPUik::Vector3{v.x,v.y,v.z} *to_bepu_scale;
#endif
}
static Vector3 from_bepu_vector3(const BEPUik::Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return Vector3{v.Z,v.Y,v.X} *to_pragma_scale;
#else
	return Vector3{v.x,v.y,v.z} *to_pragma_scale;
#endif
}
static BEPUik::Vector3 to_bepu_axis(const Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return BEPUik::Vector3{v.z,v.y,v.x};
#else
	return BEPUik::Vector3{v.x,v.y,v.z};
#endif
}
static Vector3 from_bepu_axis(const BEPUik::Vector3 &v)
{
#ifdef SWITCH_HANDEDNESS
	return Vector3{v.Z,v.Y,v.X};
#else
	return Vector3{v.x,v.y,v.z};
#endif
}
static BEPUik::Quaternion to_bepu_quaternion(const Quat &r)
{
#ifdef SWITCH_HANDEDNESS
	return BEPUik::Quaternion{-r.w,r.z,r.y,r.x};
#else
	return BEPUik::quaternion::Create(r.x,r.y,r.z,r.w);
#endif
}
static Quat from_bepu_quaternion(const BEPUik::Quaternion &r)
{
#ifdef SWITCH_HANDEDNESS
	return Quat{-r.W,r.Z,r.Y,r.X};
#else
	return Quat{r.w,r.x,r.y,r.z};
#endif
	
}
static float to_bepu_length(float l)
{
	return l *to_bepu_scale;
}
static float from_bepu_length(float l)
{
	return l *to_pragma_scale;
}

pragma::ik::Bone::Bone(const Vector3 &pos,const Quat &rot,float radius,float length,float mass)
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
Vector3 pragma::ik::Bone::GetPos() const {
	return from_bepu_vector3(m_bone->Position);
}
Quat pragma::ik::Bone::GetRot() const {
	return from_bepu_quaternion(m_bone->Orientation);
}
void pragma::ik::Bone::SetPos(const Vector3 &pos) const {
	m_bone->Position = to_bepu_vector3(pos);
}
void pragma::ik::Bone::SetRot(const Quat &rot) const {
	m_bone->Orientation = to_bepu_quaternion(rot);
}
void pragma::ik::Bone::SetPinned(bool pinned) {m_bone->Pinned = pinned;}
bool pragma::ik::Bone::IsPinned() {return m_bone->Pinned;}
BEPUik::Bone *pragma::ik::Bone::operator*() {return m_bone.get();}
BEPUik::Bone *pragma::ik::Bone::operator->() {return m_bone.get();}


pragma::ik::IControl::~IControl() {}
BEPUik::Control *pragma::ik::IControl::operator*() {return m_control.get();}

pragma::ik::IControl::IControl() {}

const BEPUik::SingleBoneLinearMotor &pragma::ik::ILinearMotorControl::GetLinearMotor() const {return const_cast<ILinearMotorControl*>(this)->GetLinearMotor();}
void pragma::ik::ILinearMotorControl::SetTargetPosition(const Vector3 &pos)
{
	GetLinearMotor().TargetPosition = to_bepu_vector3(pos);
}
Vector3 pragma::ik::ILinearMotorControl::GetTargetPosition() const {
	return from_bepu_vector3(GetLinearMotor().TargetPosition);
}

pragma::ik::DragControl::DragControl(Bone &bone)
{
	auto ctrl = std::make_unique<BEPUik::DragControl>();
	ctrl->SetTargetBone(*bone);
	m_control = std::move(ctrl);
}
pragma::ik::DragControl::~DragControl() {}
BEPUik::SingleBoneLinearMotor &pragma::ik::DragControl::GetLinearMotor() {return *static_cast<BEPUik::DragControl*>(m_control.get())->GetLinearMotor();}
	
pragma::ik::AngularPlaneControl::AngularPlaneControl(Bone &bone)
{
	auto ctrl = std::make_unique<BEPUik::AngularPlaneControl>();
	ctrl->SetTargetBone(*bone);
	m_control = std::move(ctrl);
}
pragma::ik::AngularPlaneControl::~AngularPlaneControl() {}
void pragma::ik::AngularPlaneControl::SetPlaneNormal(const Vector3 &n)
{
	GetAngularMotor().PlaneNormal = to_bepu_axis(n);
}
Vector3 pragma::ik::AngularPlaneControl::GetPlaneNormal() const {
	return from_bepu_axis(GetAngularMotor().PlaneNormal);
}
void pragma::ik::AngularPlaneControl::SetBoneLocalAxis(const Vector3 &n)
{
	GetAngularMotor().BoneLocalAxis = to_bepu_axis(n);
}
Vector3 pragma::ik::AngularPlaneControl::GetBoneLocalAxis() const {
	return from_bepu_axis(GetAngularMotor().BoneLocalAxis);
}

BEPUik::SingleBoneAngularPlaneConstraint &pragma::ik::AngularPlaneControl::GetAngularMotor() {return *static_cast<BEPUik::AngularPlaneControl*>(m_control.get())->GetAngularMotor();}
const BEPUik::SingleBoneAngularPlaneConstraint &pragma::ik::AngularPlaneControl::GetAngularMotor() const {return const_cast<AngularPlaneControl*>(this)->GetAngularMotor();}
	
pragma::ik::StateControl::StateControl(Bone &bone)
{
	auto ctrl = std::make_unique<BEPUik::StateControl>();
	ctrl->SetTargetBone(*bone);
	m_control = std::move(ctrl);
}
pragma::ik::StateControl::~StateControl() {}
void pragma::ik::StateControl::SetTargetOrientation(const Quat &rot)
{
	static_cast<BEPUik::StateControl*>(m_control.get())->GetAngularMotor()->TargetOrientation = to_bepu_quaternion(rot);
}
Quat pragma::ik::StateControl::GetTargetOrientation() const {
	return from_bepu_quaternion(static_cast<BEPUik::StateControl*>(m_control.get())->GetAngularMotor()->TargetOrientation);
}
BEPUik::SingleBoneLinearMotor &pragma::ik::StateControl::GetLinearMotor() {return *static_cast<BEPUik::StateControl*>(m_control.get())->GetLinearMotor();}
	
pragma::ik::IJoint::~IJoint() {}

void pragma::ik::IJoint::SetRigidity(float rigidity) {
	m_joint->SetRigidity(rigidity);
}
float pragma::ik::IJoint::GetRigidity() {
	return m_joint->GetRigidity();
}

BEPUik::IKJoint *pragma::ik::IJoint::operator*() {return m_joint.get();}
	
pragma::ik::IJoint::IJoint() {}
	
pragma::ik::DistanceJoint::DistanceJoint(Bone &bone0,Bone &bone1)
{
	auto joint = std::make_unique<BEPUik::IKDistanceJoint>(
		**bone0,**bone1,
		bone0->Position,bone1->Position
	);
	m_joint = std::move(joint);
}
pragma::ik::DistanceJoint::~DistanceJoint() {}
	
pragma::ik::PointOnLineJoint::PointOnLineJoint(Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,const Vector3 &anchorB)
{
	auto joint = std::make_unique<BEPUik::IKPointOnLineJoint>(
		**bone0,**bone1,
		to_bepu_vector3(lineAnchor),
		to_bepu_axis(lineDirection),
		to_bepu_vector3(anchorB)
	);
	m_joint = std::move(joint);
}
pragma::ik::PointOnLineJoint::~PointOnLineJoint() {}
	
pragma::ik::BallSocketJoint::BallSocketJoint(Bone &bone0,Bone &bone1,const Vector3 &anchor)
{
	auto joint = std::make_unique<BEPUik::IKBallSocketJoint>(
		**bone0,**bone1,
		to_bepu_vector3(anchor)
	);
	m_joint = std::move(joint);
}
pragma::ik::BallSocketJoint::~BallSocketJoint() {}
	
pragma::ik::AngularJoint::AngularJoint(Bone &bone0,Bone &bone1)
{
	auto joint = std::make_unique<BEPUik::IKAngularJoint>(
		**bone0,**bone1
	);
	m_joint = std::move(joint);
}
pragma::ik::AngularJoint::~AngularJoint() {}
	
pragma::ik::RevoluteJoint::RevoluteJoint(Bone &bone0,Bone &bone1,const Vector3 &freeAxis)
{
	auto joint = std::make_unique<BEPUik::IKRevoluteJoint>(
		**bone0,**bone1,
		to_bepu_axis(freeAxis)
	);
	m_joint = std::move(joint);
}
pragma::ik::RevoluteJoint::~RevoluteJoint() {}
	
pragma::ik::TwistJoint::TwistJoint(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB)
{
	auto joint = std::make_unique<BEPUik::IKTwistJoint>(
		**bone0,**bone1,
		to_bepu_axis(axisA),to_bepu_axis(axisB)
	);
	m_joint = std::move(joint);
}
pragma::ik::TwistJoint::~TwistJoint() {}
	
pragma::ik::SwingLimit::SwingLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle)
{
	auto joint = std::make_unique<BEPUik::IKSwingLimit>(
		**bone0,**bone1,
		to_bepu_axis(axisA),
		to_bepu_axis(axisB),
		maxAngle
	);
	m_joint = std::move(joint);
}
pragma::ik::SwingLimit::~SwingLimit() {}
	
	
pragma::ik::EllipseSwingLimit::EllipseSwingLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,const Vector3 &xAxis,float maxAngleX,float maxAngleY)
{
	auto joint = std::make_unique<BEPUik::IKEllipseSwingLimit>(
		**bone0,**bone1,
		to_bepu_axis(axisA),
		to_bepu_axis(axisB),
		to_bepu_axis(xAxis),
		maxAngleX,maxAngleY
	);
	m_joint = std::move(joint);
}
pragma::ik::EllipseSwingLimit::~EllipseSwingLimit() {}
	
	
pragma::ik::LinearAxisLimit::LinearAxisLimit(Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,const Vector3 &anchorB,float minimumDistance,float maximumDistance)
{
	auto joint = std::make_unique<BEPUik::IKLinearAxisLimit>(
		**bone0,**bone1,
		to_bepu_vector3(lineAnchor),
		to_bepu_axis(lineDirection),
		to_bepu_vector3(anchorB),
		to_bepu_length(minimumDistance),
		to_bepu_length(maximumDistance)
	);
	m_joint = std::move(joint);
}
pragma::ik::LinearAxisLimit::~LinearAxisLimit() {}
	
	
pragma::ik::TwistLimit::TwistLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle)
{
	auto joint = std::make_unique<BEPUik::IKTwistLimit>(
		**bone0,**bone1,
		to_bepu_axis(axisA),
		to_bepu_axis(axisB),
		maxAngle
	);
	joint->ComputeMeasurementAxes();
	m_joint = std::move(joint);
}
void pragma::ik::TwistLimit::SetMeasurementAxisA(const Vector3 &axis) {
	static_cast<BEPUik::IKTwistLimit*>(m_joint.get())->SetMeasurementAxisA(to_bepu_axis(axis));
}
Vector3 pragma::ik::TwistLimit::GetMeasurementAxisA() {return from_bepu_axis(static_cast<BEPUik::IKTwistLimit*>(m_joint.get())->GetMeasurementAxisA());}
void pragma::ik::TwistLimit::SetMeasurementAxisB(const Vector3 &axis) {
	static_cast<BEPUik::IKTwistLimit*>(m_joint.get())->SetMeasurementAxisB(to_bepu_axis(axis));
}
Vector3 pragma::ik::TwistLimit::GetMeasurementAxisB() {return from_bepu_axis(static_cast<BEPUik::IKTwistLimit*>(m_joint.get())->GetMeasurementAxisB());}
pragma::ik::TwistLimit::~TwistLimit() {}
	
	
pragma::ik::SwivelHingeJoint::SwivelHingeJoint(Bone &bone0,Bone &bone1,const Vector3 &worldHingeAxis,const Vector3 &worldTwistAxis)
{
	auto joint = std::make_unique<BEPUik::IKSwivelHingeJoint>(
		**bone0,**bone1,
		to_bepu_axis(worldHingeAxis),to_bepu_axis(worldTwistAxis)
	);
	m_joint = std::move(joint);
}
pragma::ik::SwivelHingeJoint::~SwivelHingeJoint() {}
	
	
pragma::ik::Solver::Solver(uint32_t controlIterationCount,uint32_t fixerIterationCount)
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
void pragma::ik::Solver::Solve()
{
	m_solver->Solve(m_bepuControls);
}
pragma::ik::DragControl &pragma::ik::Solver::AddDragControl(Bone &bone)
{
	auto ctrl = std::make_shared<DragControl>(bone);
	m_controls.push_back(ctrl);
	m_bepuControls.push_back(**ctrl);
	return *ctrl;
}
void pragma::ik::Solver::RemoveDragControl(const IControl &ctrl)
{
	auto it = std::find_if(m_controls.begin(),m_controls.end(),[&ctrl](const std::shared_ptr<IControl> &other) {
		return &ctrl == other.get();
	});
	if(it == m_controls.end())
		return;
	auto idx = it -m_controls.begin();
	m_controls.erase(it);
	m_bepuControls.erase(m_bepuControls.begin() +idx);
}
pragma::ik::AngularPlaneControl &pragma::ik::Solver::AddAngularPlaneControl(Bone &bone)
{
	auto ctrl = std::make_shared<AngularPlaneControl>(bone);
	m_controls.push_back(ctrl);
	m_bepuControls.push_back(**ctrl);
	return *ctrl;
}
pragma::ik::StateControl &pragma::ik::Solver::AddStateControl(Bone &bone)
{
	auto ctrl = std::make_shared<StateControl>(bone);
	m_controls.push_back(ctrl);
	m_bepuControls.push_back(**ctrl);
	return *ctrl;
}
pragma::ik::DistanceJoint &pragma::ik::Solver::AddDistanceJoint(Bone &bone0,Bone &bone1)
{
	auto joint = std::make_shared<DistanceJoint>(bone0,bone1);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::BallSocketJoint &pragma::ik::Solver::AddBallSocketJoint(Bone &bone0,Bone &bone1,const Vector3 &anchor)
{
	auto joint = std::make_shared<BallSocketJoint>(bone0,bone1,anchor);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::AngularJoint &pragma::ik::Solver::AddAngularJoint(Bone &bone0,Bone &bone1)
{
	auto joint = std::make_shared<AngularJoint>(bone0,bone1);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::PointOnLineJoint &pragma::ik::Solver::AddPointOnLineJoint(Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,const Vector3 &anchorB)
{
	auto joint = std::make_shared<PointOnLineJoint>(bone0,bone1,lineAnchor,lineDirection,anchorB);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::RevoluteJoint &pragma::ik::Solver::AddRevoluteJoint(Bone &bone0,Bone &bone1,const Vector3 &freeAxis)
{
	auto joint = std::make_shared<RevoluteJoint>(bone0,bone1,freeAxis);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::SwingLimit &pragma::ik::Solver::AddSwingLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle)
{
	auto joint = std::make_shared<SwingLimit>(bone0,bone1,axisA,axisB,maxAngle);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::EllipseSwingLimit &pragma::ik::Solver::AddEllipseSwingLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,const Vector3 &xAxis,float maxAngleX,float maxAngleY)
{
	auto joint = std::make_shared<EllipseSwingLimit>(bone0,bone1,axisA,axisB,xAxis,maxAngleX,maxAngleY);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::LinearAxisLimit &pragma::ik::Solver::AddLinearAxisLimit(Bone &bone0,Bone &bone1,const Vector3 &lineAnchor,const Vector3 &lineDirection,const Vector3 &anchorB,float minimumDistance,float maximumDistance)
{
	auto joint = std::make_shared<LinearAxisLimit>(bone0,bone1,lineAnchor,lineDirection,anchorB,minimumDistance,maximumDistance);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::TwistJoint &pragma::ik::Solver::AddTwistJoint(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB)
{
	auto joint = std::make_shared<TwistJoint>(bone0,bone1,axisA,axisB);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::TwistLimit &pragma::ik::Solver::AddTwistLimit(Bone &bone0,Bone &bone1,const Vector3 &axisA,const Vector3 &axisB,float maxAngle)
{
	auto joint = std::make_shared<TwistLimit>(bone0,bone1,axisA,axisB,maxAngle);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::SwivelHingeJoint &pragma::ik::Solver::AddSwivelHingeJoint(Bone &bone0,Bone &bone1,const Vector3 &worldHingeAxis,const Vector3 &worldTwistAxis)
{
	auto joint = std::make_shared<SwivelHingeJoint>(bone0,bone1,worldHingeAxis,worldTwistAxis);
	m_joints.push_back(joint);
	m_bepuJoints.push_back(**joint);
	return *joint;
}
pragma::ik::Bone &pragma::ik::Solver::AddBone(const Vector3 &pos,const Quat &rot,float radius,float length)
{
	auto bone = std::make_shared<Bone>(pos,rot,radius,length,1.f);
	m_bones.push_back(bone);
	m_bepuBones.push_back(**bone);
	return *bone;
}

size_t pragma::ik::Solver::GetControlCount() const {return m_controls.size();}
size_t pragma::ik::Solver::GetBoneCount() const {return m_bones.size();}
size_t pragma::ik::Solver::GetJointCount() const {return m_joints.size();}

pragma::ik::IControl *pragma::ik::Solver::GetControl(size_t index) {return (index < m_controls.size()) ? m_controls[index].get() : nullptr;}
pragma::ik::Bone *pragma::ik::Solver::GetBone(size_t index) {return (index < m_bones.size()) ? m_bones[index].get() : nullptr;}
pragma::ik::IJoint *pragma::ik::Solver::GetJoint(size_t index) {return (index < m_joints.size()) ? m_joints[index].get() : nullptr;}

const std::vector<std::shared_ptr<pragma::ik::IControl>> &pragma::ik::Solver::GetControls() const {return m_controls;}
const std::vector<std::shared_ptr<pragma::ik::Bone>> &pragma::ik::Solver::GetBones() const {return m_bones;}
const std::vector<std::shared_ptr<pragma::ik::IJoint>> &pragma::ik::Solver::GetJoints() const {return m_joints;}
