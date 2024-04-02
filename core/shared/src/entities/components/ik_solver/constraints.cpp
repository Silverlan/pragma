/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/ik_solver_component.hpp"

using namespace pragma;

static void clamp_angles(float &min, float &max)
{
	// If the span range is too small it can cause instability,
	// so we'll force a minimum span angle
	constexpr umath::Degree minSpan = 0.5f;
	if(umath::abs(max - min) < minSpan) {
		auto baseAngle = (min + max) / 2.f;
		min = baseAngle - minSpan;
		max = baseAngle + minSpan;
	}
}
static void init_joint(const pragma::IkSolverComponent::ConstraintInfo &constraintInfo, pragma::ik::IJoint &joint)
{
	auto maxForce = (constraintInfo.maxForce < 0.f) ? std::numeric_limits<float>::max() : constraintInfo.maxForce;
	joint.SetRigidity(constraintInfo.rigidity);
	joint.SetMaxForce(maxForce);
}
void IkSolverComponent::AddFixedConstraint(const ConstraintInfo &constraintInfo)
{
	pragma::ik::Bone *bone0, *bone1;
	umath::ScaledTransform refPose0, refPose1;
	if(!GetConstraintBones(constraintInfo.boneId0, constraintInfo.boneId1, &bone0, &bone1, refPose0, refPose1))
		return;
	auto &rotBone0 = refPose0.GetRotation();
	auto &rotBone1 = refPose1.GetRotation();

	// Lock distance and rotation to the parent
	auto &bsJoint = m_ikSolver->AddBallSocketJoint(*bone0, *bone1, bone1->GetPos());
	init_joint(constraintInfo, bsJoint);

	// Lock the angles
	auto &joint = m_ikSolver->AddAngularJoint(*bone0, *bone1);
	init_joint(constraintInfo, joint);
}
void IkSolverComponent::AddHingeConstraint(const ConstraintInfo &constraintInfo, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation, SignedAxis etwistAxis)
{
	pragma::ik::Bone *bone0, *bone1;
	umath::ScaledTransform refPose0, refPose1;
	if(!GetConstraintBones(constraintInfo.boneId0, constraintInfo.boneId1, &bone0, &bone1, refPose0, refPose1))
		return;
	auto &rotBone0 = refPose0.GetRotation();
	auto &rotBone1 = refPose1.GetRotation();

	auto twistRotOffset = uquat::identity();

	// If the twist axis is NOT the X axis, we'll have to rotate
	// the main axis around a bit and adjust the limits accordingly.
	switch(etwistAxis) {
	case pragma::SignedAxis::X:
	case pragma::SignedAxis::NegX:
		break;
	case pragma::SignedAxis::Y:
	case pragma::SignedAxis::NegY:
		twistRotOffset = uquat::create(EulerAngles(0.0, 0.f, 90.f));
		break;
	case pragma::SignedAxis::Z:
	case pragma::SignedAxis::NegZ:
		twistRotOffset = uquat::create(EulerAngles(0.0, 90.f, 0.f));
		umath::swap(minAngle, maxAngle);
		minAngle *= -1.f;
		maxAngle *= -1.f;
		break;
	}

	rotBone0 *= twistRotOffset;
	rotBone1 *= twistRotOffset;

	// The IK system only allows us to specify a general swing limit (in any direction). Since we want to be able to specify it in each
	// direction independently, we have to shift the rotation axes accordingly.
	umath::swap(minAngle, maxAngle);
	minAngle *= -0.5f;
	maxAngle *= -0.5f;
	clamp_angles(minAngle, maxAngle);
	auto rotBone1WithOffset = rotBone1 * uquat::create(EulerAngles(-(maxAngle + minAngle), 0, 0));

	// BallSocket is required to ensure the distance and rotation to the parent is locked
	auto &bsJoint = m_ikSolver->AddBallSocketJoint(*bone0, *bone1, bone1->GetPos());
	init_joint(constraintInfo, bsJoint);

	// Revolute joint to lock rotation to a single axis
	auto &revJoint = m_ikSolver->AddRevoluteJoint(*bone0, *bone1, uquat::right(rotBone1));
	init_joint(constraintInfo, revJoint);

	// Apply the swing limit
	auto &swingLimit = m_ikSolver->AddSwingLimit(*bone0, *bone1, uquat::up(rotBone1WithOffset), uquat::up(rotBone1), umath::deg_to_rad(maxAngle - minAngle));
	init_joint(constraintInfo, swingLimit);
}

void IkSolverComponent::AddBallSocketConstraint(const ConstraintInfo &constraintInfo, const EulerAngles &minLimits, const EulerAngles &maxLimits, SignedAxis etwistAxis)
{
	pragma::ik::Bone *bone0, *bone1;
	umath::ScaledTransform refPose0, refPose1;
	if(!GetConstraintBones(constraintInfo.boneId0, constraintInfo.boneId1, &bone0, &bone1, refPose0, refPose1))
		return;
	// BallSocket is required to ensure the distance and rotation to the parent is locked
	auto &bsJoint = m_ikSolver->AddBallSocketJoint(*bone0, *bone1, bone1->GetPos());
	init_joint(constraintInfo, bsJoint);

	auto effectiveMinLimits = minLimits * 0.5f;
	auto effectiveMaxLimits = maxLimits * 0.5f;

	std::array<bool, 3> limitEnabled = {};
	for(uint8_t i = 0; i < 3; ++i) {
		// We'll consider the limit to be disabled if the min value is larger than the max value
		limitEnabled[i] = effectiveMaxLimits[i] >= effectiveMinLimits[i];
		if(!limitEnabled[i]) {
			effectiveMaxLimits[i] = 0.f;
			effectiveMinLimits[i] = 0.f;
		}
	}

	for(uint8_t i = 0; i < 3; ++i)
		clamp_angles(effectiveMinLimits[i], effectiveMaxLimits[i]);

	auto &rotBone0 = refPose0.GetRotation();
	auto &rotBone1 = refPose1.GetRotation();

	// If the twist axis is NOT the Z axis, we'll have to rotate
	// the main axis around a bit and adjust the limits accordingly.
	auto twistRotOffset = Model::GetTwistAxisRotationOffset(etwistAxis);
	uquat::inverse(twistRotOffset);

	auto refRot1 = rotBone1 * twistRotOffset;

	auto useEllipseSwingLimit = true;
	if(umath::abs((effectiveMaxLimits.p - effectiveMinLimits.p) - (effectiveMaxLimits.y - effectiveMinLimits.y)) <= 0.01f) {
		// Swing limits are the same on both axes, so we can use a simple swing limit (which is less expensive)
		useEllipseSwingLimit = false;
	}

	if(!useEllipseSwingLimit) {
		// Convert ellipse swing limit to general swing limit
		auto maxSpan = umath::max(effectiveMaxLimits.p - effectiveMinLimits.p, effectiveMaxLimits.y - effectiveMinLimits.y);
		auto pMid = (effectiveMaxLimits.p + effectiveMinLimits.p) * 0.5f;
		effectiveMaxLimits.p = pMid + maxSpan * 0.5f;
		effectiveMinLimits.p = pMid - maxSpan * 0.5f;
		auto yMid = (effectiveMaxLimits.y + effectiveMinLimits.y) * 0.5f;
		effectiveMaxLimits.y = yMid + maxSpan * 0.5f;
		effectiveMinLimits.y = yMid - maxSpan * 0.5f;
	}

	auto effectiveLimitSpan = effectiveMinLimits + effectiveMaxLimits;
	float effectiveTwistSpan;

	auto enableTwistLimit = true;
	auto enableSwingLimit = true;
	// Eliminate rotation around twist axis (twist is handled separately using a twist limit, see further below)
	switch(etwistAxis) {
	case SignedAxis::X:
	case SignedAxis::NegX:
		effectiveTwistSpan = effectiveLimitSpan.p;
		effectiveLimitSpan.p = 0.f;
		enableTwistLimit = limitEnabled[0];
		enableSwingLimit = limitEnabled[1] && limitEnabled[2];
		break;
	case SignedAxis::Y:
	case SignedAxis::NegY:
		effectiveTwistSpan = effectiveLimitSpan.y;
		effectiveLimitSpan.y = 0.f;
		enableTwistLimit = limitEnabled[1];
		enableSwingLimit = limitEnabled[0] && limitEnabled[2];
		break;
	case SignedAxis::Z:
	case SignedAxis::NegZ:
		effectiveTwistSpan = effectiveLimitSpan.r;
		effectiveLimitSpan.r = 0.f;
		enableTwistLimit = limitEnabled[2];
		enableSwingLimit = limitEnabled[0] && limitEnabled[1];
		break;
	}

	auto twistAxis = uquat::forward(twistRotOffset);
	// We need to rotate the axis for cases where minLimitAngle != -maxLimitAngle, since the axis will be off-center in those cases
	uvec::rotate(&twistAxis, uquat::create(effectiveLimitSpan));
	auto q1 = twistAxis;
	twistAxis = rotBone1 * twistAxis;
	auto &axisA = twistAxis;

	// Depending on the twist axis, we have to switch around the angle limits
	if(etwistAxis == SignedAxis::X || etwistAxis == SignedAxis::NegX) {
		effectiveMinLimits = {effectiveMinLimits.r, effectiveMinLimits.y, effectiveMinLimits.p};
		effectiveMaxLimits = {effectiveMaxLimits.r, effectiveMaxLimits.y, effectiveMaxLimits.p};
	}
	if(etwistAxis == SignedAxis::Y || etwistAxis == SignedAxis::NegY) {
		effectiveMinLimits = {effectiveMinLimits.p, effectiveMinLimits.r, effectiveMinLimits.y};
		effectiveMaxLimits = {effectiveMaxLimits.p, effectiveMaxLimits.r, effectiveMaxLimits.y};
	}
	if(etwistAxis == SignedAxis::Z || etwistAxis == SignedAxis::NegZ) {
		// No need to do anything
	}

	auto axisB = uquat::forward(refRot1);
	auto axisBRight = uquat::right(refRot1);
	auto axisBUp = uquat::up(refRot1);
	auto span = effectiveMaxLimits.y - effectiveMinLimits.y; // TODO: Use enableTwistLimit to check if the limit is enabled
	if(enableSwingLimit && span >= 0.f && span < 179.99f) {
		if(!useEllipseSwingLimit) {
			auto &ellipseSwingLimit = m_ikSolver->AddSwingLimit(*bone0, *bone1, axisA, axisB, umath::deg_to_rad(effectiveMaxLimits.y - effectiveMinLimits.y));
			init_joint(constraintInfo, ellipseSwingLimit);
		}
		else {
			auto &ellipseSwingLimit = m_ikSolver->AddEllipseSwingLimit(*bone0, *bone1, axisA, axisB, axisBRight, axisBUp, umath::deg_to_rad(effectiveMaxLimits.y - effectiveMinLimits.y), umath::deg_to_rad(effectiveMaxLimits.p - effectiveMinLimits.p));
			init_joint(constraintInfo, ellipseSwingLimit);
		}
	}

	// Twist motion
	if(enableTwistLimit) {
		auto twistLimitVal = effectiveTwistSpan;
		if(twistLimitVal >= 0.f && twistLimitVal < 179.99f) {
			// Note: Using a different max/min angle limit for twist rotation is currently not supported
			auto &twistLimit = m_ikSolver->AddTwistLimit(*bone0, *bone1, axisA, axisB, umath::deg_to_rad(twistLimitVal));
			init_joint(constraintInfo, twistLimit);
		}
	}
}
