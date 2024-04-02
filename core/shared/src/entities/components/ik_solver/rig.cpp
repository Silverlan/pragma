/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/ik_solver_component.hpp"
#include "pragma/model/model.h"
#include "pragma/logging.hpp"

using namespace pragma;

void IkSolverComponent::SetIkRigFile(const std::string &RigConfigFile)
{
	m_ikRigFile = RigConfigFile;
	UpdateIkRigFile();
}
const std::string &IkSolverComponent::GetIkRigFile() const { return m_ikRigFile; }
void IkSolverComponent::UpdateIkRigFile()
{
	if(!m_ikRigFile.empty())
		m_ikRig = udm::Property::Create<udm::Element>();
	InitializeSolver(); // Clear Rig
	if(!m_ikRigFile.empty()) {
		auto rigConfig = pragma::ik::RigConfig::load("scripts/ik_rigs/" + m_ikRigFile);
		if(rigConfig)
			AddIkSolverByRig(*rigConfig);
	}
	UpdateIkRig();
}
bool IkSolverComponent::AddIkSolverByChain(const std::string &boneName, uint32_t chainLength)
{
	constexpr uint32_t minChainLength = 3;
	if(chainLength < minChainLength) {
		spdlog::debug("Failed to add ik chain to ik solver {} with boneName={} and chainLength={}: Chain length has to be at least {}.", GetEntity().ToString(), boneName, chainLength, minChainLength);
		return false;
	}
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(!mdl) {
		spdlog::debug("Failed to add ik chain to ik solver {} with boneName={} and chainLength={}: Entity has no model.", GetEntity().ToString(), boneName, chainLength);
		return false;
	}
	auto &skeleton = mdl->GetSkeleton();
	auto &ref = mdl->GetReference();
	std::vector<pragma::animation::BoneId> ikChain;
	ikChain.reserve(chainLength);
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId == -1) {
		spdlog::debug("Failed to add ik chain to ik solver {} with boneName={} and chainLength={}: Bone not found in skeleton.", GetEntity().ToString(), boneName, chainLength);
		return false;
	}

	auto bone = skeleton.GetBone(boneId).lock();
	for(auto i = decltype(chainLength) {0}; i < chainLength; ++i) {
		if(bone == nullptr) {
			spdlog::debug("Failed to add ik chain to ik solver {} with boneName={} and chainLength={}: Chain length exceeds number of parents.", GetEntity().ToString(), boneName, chainLength);
			return false;
		}
		ikChain.insert(ikChain.begin(), bone->ID);
		bone = bone->parent.lock();
	}

	auto rig = pragma::ik::RigConfig();
	for(auto id : ikChain) {
		auto bone = skeleton.GetBone(id).lock();
		assert(bone != nullptr);
		rig.AddBone(bone->name);
	}

	// Pin the top-most parent of the chain (e.g. shoulder)
	rig.SetBoneLocked(skeleton.GetBone(ikChain.front()).lock()->name, true);

	// Add handles for all other bones in the chain (e.g. forearm or hand)
	for(auto i = decltype(ikChain.size()) {2u}; i < ikChain.size(); ++i) {
		// We want to be able to control the rotation of the last element in the chain (the effector), but
		// not the other elements
		if(i == ikChain.size() - 1) {
			auto ctrl = rig.AddControl(skeleton.GetBone(ikChain[i]).lock()->name, pragma::ik::RigConfigControl::Type::OrientedDrag);
			if(ctrl)
				ctrl->rigidity = 10.f;
		}
		else {
			auto ctrl = rig.AddControl(skeleton.GetBone(ikChain[i]).lock()->name, pragma::ik::RigConfigControl::Type::Drag);
			if(ctrl)
				ctrl->rigidity = 5.f;
		}
	}

	// Inverted limits will ensure that limits are disabled
	constexpr EulerAngles maxLimits {-1.f, -1.f, -1.f};
	constexpr EulerAngles minLimits {1.f, 1.f, 1.f};

	// Add generic ballsocket constraints with no twist
	for(auto i = decltype(ikChain.size()) {1u}; i < ikChain.size(); ++i) {
		// We need to allow some minor twisting to avoid instability
		rig.AddBallSocketConstraint(skeleton.GetBone(ikChain[i - 1]).lock()->name, skeleton.GetBone(ikChain[i]).lock()->name, minLimits, maxLimits);
	}
	if(!AddIkSolverByRig(rig))
		return false;
	udm::LinkedPropertyWrapper prop {*m_ikRig};
	rig.ToUdmData(prop);
	return true;
}
bool IkSolverComponent::AddIkSolverByRig(const pragma::ik::RigConfig &rigConfig)
{
	auto mdl = GetEntity().GetModel();
	if(!mdl) {
		spdlog::debug("Failed to add ik rig to ik solver {}: Entity has no model.", GetEntity().ToString());
		return false;
	}
	auto &skeleton = mdl->GetSkeleton();

	using HierarchyDepth = uint8_t;
	struct BoneInfo {
		uint32_t rigBoneIndex;
		pragma::animation::BoneId skeletonBoneIndex;
		HierarchyDepth hierarchyDepth;

		const pragma::ik::RigConfigBone *configBone = nullptr;
	};
	std::vector<BoneInfo> bones;
	auto &rigBones = rigConfig.GetBones();
	bones.reserve(rigBones.size());

	uint32_t rigBoneIdx = 0;
	for(auto &boneData : rigBones) {
		auto boneId = skeleton.LookupBone(boneData->name);
		if(boneId == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Bone {} does not exist in skeleton.", GetEntity().ToString(), boneData->name.str);
			return false;
		}
		HierarchyDepth depth = 0;
		auto bone = skeleton.GetBone(boneId);
		assert(!bone.expired());
		auto parent = bone.lock()->parent;
		while(!parent.expired()) {
			++depth;
			parent = parent.lock()->parent;
		}
		bones.push_back(BoneInfo {rigBoneIdx++, static_cast<pragma::animation::BoneId>(boneId), depth, boneData.get()});
	}

	auto &ref = mdl->GetReference();

	// Sort the bones to be in hierarchical order. This is not necessary for the ik solver, but the order is important
	// when the animation is updated.
	std::sort(bones.begin(), bones.end(), [](const BoneInfo &a, const BoneInfo &b) { return a.hierarchyDepth < b.hierarchyDepth; });
	for(auto i = decltype(bones.size()) {0u}; i < bones.size(); ++i) {
		auto &boneInfo = bones[i];
		auto bone = skeleton.GetBone(boneInfo.skeletonBoneIndex).lock();
		if(!bone)
			continue;
		umath::ScaledTransform pose {};
		ref.GetBonePose(boneInfo.skeletonBoneIndex, pose);
		auto &rigBone = rigBones[boneInfo.rigBoneIndex];
		float radius = 1.f;
		float length = 1.f;
		if(boneInfo.configBone) {
			radius = boneInfo.configBone->length;
			length = boneInfo.configBone->width;
			if(boneInfo.configBone->ikPose)
				pose = *boneInfo.configBone->ikPose;
		}

		auto *ikBone = AddBone(bone->name, boneInfo.skeletonBoneIndex, pose, radius, length);
		if(rigBone->locked)
			SetBoneLocked(boneInfo.skeletonBoneIndex, true);
	}

	for(auto &controlData : rigConfig.GetControls()) {
		auto boneId = skeleton.LookupBone(controlData->bone);
		if(boneId == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Control bone {} does not exist in skeleton.", GetEntity().ToString(), controlData->bone.str);
			return false;
		}
		AddControl(boneId, controlData->type, controlData->maxForce, controlData->rigidity);
	}

	for(auto &constraintData : rigConfig.GetConstraints()) {
		auto boneId0 = skeleton.LookupBone(constraintData->bone0);
		auto boneId1 = skeleton.LookupBone(constraintData->bone1);
		if(boneId0 == -1 || boneId1 == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Constraint bone {} or {} does not exist in skeleton.", GetEntity().ToString(), constraintData->bone0.str, constraintData->bone1.str);
			return false;
		}

		ConstraintInfo constraintInfo {static_cast<pragma::animation::BoneId>(boneId0), static_cast<pragma::animation::BoneId>(boneId1)};
		constraintInfo.rigidity = constraintData->rigidity;
		constraintInfo.maxForce = constraintData->maxForce;
		switch(constraintData->type) {
		case pragma::ik::RigConfigConstraint::Type::Fixed:
			AddFixedConstraint(constraintInfo);
			break;
		case pragma::ik::RigConfigConstraint::Type::Hinge:
			AddHingeConstraint(constraintInfo, constraintData->minLimits.p, constraintData->maxLimits.p, constraintData->offsetPose.GetRotation(), constraintData->axis);
			break;
		case pragma::ik::RigConfigConstraint::Type::BallSocket:
			AddBallSocketConstraint(constraintInfo, constraintData->minLimits, constraintData->maxLimits, constraintData->axis);
			break;
		}
	}

	for(auto &jointData : rigConfig.GetJoints()) {
		auto boneId0 = skeleton.LookupBone(jointData->bone0);
		auto boneId1 = skeleton.LookupBone(jointData->bone1);
		if(boneId0 == -1 || boneId1 == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Constraint bone {} or {} does not exist in skeleton.", GetEntity().ToString(), jointData->bone0.str, jointData->bone1.str);
			return false;
		}

		JointInfo jointInfo {static_cast<pragma::animation::BoneId>(boneId0), static_cast<pragma::animation::BoneId>(boneId1)};
		if(jointData->axisA)
			jointInfo.axisA = *jointData->axisA;
		if(jointData->axisB)
			jointInfo.axisB = *jointData->axisB;
		if(jointData->maxAngle)
			jointInfo.maxAngle = *jointData->maxAngle;
		jointInfo.rigidity = jointData->rigidity;
		jointInfo.anchorPosition = jointData->anchorPosition;
		jointInfo.measurementAxisA = jointData->measurementAxisA;
		switch(jointData->type) {
		case pragma::ik::RigConfigJoint::Type::BallSocketJoint:
			AddBallSocketJoint(jointInfo);
			break;
		case pragma::ik::RigConfigJoint::Type::SwingLimit:
			AddSwingLimit(jointInfo);
			break;
		case pragma::ik::RigConfigJoint::Type::TwistLimit:
			AddTwistLimit(jointInfo);
			break;
		case pragma::ik::RigConfigJoint::Type::SwivelHingeJoint:
			AddSwivelHingeJoint(jointInfo);
			break;
		case pragma::ik::RigConfigJoint::Type::TwistJoint:
			AddTwistJoint(jointInfo);
			break;
		}
	}
	return true;
}
