/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/ik_solver_component.hpp"
#include "pragma/entities/attribute_specialization_type.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/ik_solver/rig_config.hpp"
#include "pragma/entities/components/component_member_flags.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/components/constraints/constraint_manager_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/model.h"
#include "pragma/logging.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>

using namespace pragma;

ComponentEventId IkSolverComponent::EVENT_INITIALIZE_SOLVER = pragma::INVALID_COMPONENT_ID;
ComponentEventId IkSolverComponent::EVENT_ON_IK_UPDATED = pragma::INVALID_COMPONENT_ID;
IkSolverComponent::ConstraintInfo::ConstraintInfo(BoneId bone0, BoneId bone1) : boneId0 {bone0}, boneId1 {bone1} {}
void IkSolverComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_INITIALIZE_SOLVER = registerEvent("INITIALIZE_SOLVER", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_IK_UPDATED = registerEvent("ON_IK_UPDATED", ComponentEventInfo::Type::Explicit);
}
static void set_ik_rig(const ComponentMemberInfo &memberInfo, IkSolverComponent &component, const pragma::ents::Element &value) { component.UpdateIkRig(); }
static void get_ik_rig(const ComponentMemberInfo &memberInfo, IkSolverComponent &component, pragma::ents::Element &value) { value = component.GetIkRig(); }
void IkSolverComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = IkSolverComponent;

	{
		using TRigConfig = pragma::ents::Element;
		auto memberInfo = create_component_member_info<T, TRigConfig,
		  // For some reasons these don't work as lambdas (VS compiler bug?)
		  &set_ik_rig, &get_ik_rig>("rigConfig");
		registerMember(std::move(memberInfo));
	}
	{
		using TRigConfigFile = std::string;
		auto memberInfo = create_component_member_info<T, TRigConfigFile, static_cast<void (T::*)(const TRigConfigFile &)>(&T::SetIkRigFile), static_cast<const TRigConfigFile &(T::*)() const>(&T::GetIkRigFile)>("rigConfigFile", "", AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["rootPath"] = "scripts/ik_rigs/";
		metaData["extensions"] = pragma::ik::RigConfig::get_supported_extensions();
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}
	{
		using TResetSolver = bool;
		auto memberInfo = create_component_member_info<T, TResetSolver, static_cast<void (T::*)(TResetSolver)>(&T::SetResetSolver), static_cast<TResetSolver (T::*)() const>(&T::ShouldResetSolver)>("resetSolver", true);
		memberInfo.SetFlag(pragma::ComponentMemberFlags::HideInInterface);
		registerMember(std::move(memberInfo));
	}
}
IkSolverComponent::IkSolverComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_ikRig {udm::Property::Create<udm::Element>()} {}
void IkSolverComponent::UpdateGlobalSolverSettings()
{
	for(auto *nw : {pragma::get_engine()->GetServerNetworkState(), pragma::get_engine()->GetClientState()}) {
		if(!nw)
			continue;
		auto *game = nw->GetGameState();
		if(!game)
			continue;
		auto cIt = EntityCIterator<pragma::IkSolverComponent> {*game};
		for(auto &solverC : cIt)
			solverC.UpdateSolverSettings();
	}
}
void IkSolverComponent::UpdateSolverSettings()
{
	auto &solver = GetIkSolver();
	if(!solver)
		return;
	auto &game = GetGame();
	solver->SetTimeStepDuration(game.GetConVarFloat("ik_solver_time_step_duration"));
	solver->SetControlIterationCount(game.GetConVarInt("ik_solver_control_iteration_count"));
	solver->SetFixerIterationCount(game.GetConVarInt("ik_solver_fixer_iteration_count"));
	solver->SetVelocitySubIterationCount(game.GetConVarInt("ik_solver_velocity_sub_iteration_count"));
}
static std::array<uint32_t, umath::to_integral(NwStateType::Count)> g_solverCount {false, false};
void IkSolverComponent::Initialize()
{
	auto &nw = GetNetworkState();
	if(g_solverCount[umath::to_integral(nw.GetType())] == 0) {
		++g_solverCount[umath::to_integral(nw.GetType())];

		// Initialize change callbacks for this network state
		nw.RegisterConVarCallback("ik_solver_time_step_duration", std::function<void(NetworkState *, const ConVar &, float, float)> {+[](NetworkState *state, const ConVar &cvar, float oldVal, float newVal) { UpdateGlobalSolverSettings(); }});
		nw.RegisterConVarCallback("ik_solver_control_iteration_count", std::function<void(NetworkState *, const ConVar &, int32_t, int32_t)> {+[](NetworkState *state, const ConVar &cvar, int32_t oldVal, int32_t newVal) { UpdateGlobalSolverSettings(); }});
		nw.RegisterConVarCallback("ik_solver_fixer_iteration_count", std::function<void(NetworkState *, const ConVar &, int32_t, int32_t)> {+[](NetworkState *state, const ConVar &cvar, int32_t oldVal, int32_t newVal) { UpdateGlobalSolverSettings(); }});
		nw.RegisterConVarCallback("ik_solver_velocity_sub_iteration_count", std::function<void(NetworkState *, const ConVar &, int32_t, int32_t)> {+[](NetworkState *state, const ConVar &cvar, int32_t oldVal, int32_t newVal) { UpdateGlobalSolverSettings(); }});
	}

	BaseEntityComponent::Initialize();
	GetEntity().AddComponent<ConstraintManagerComponent>();
	auto constraintC = GetEntity().AddComponent<ConstraintComponent>();
	constraintC->SetDrivenObject(pragma::EntityUComponentMemberRef {GetEntity(), "ik_solver", "rigConfig"});
	BindEvent(ConstraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		Solve();
		return util::EventReply::Unhandled;
	});
}
IkSolverComponent::~IkSolverComponent() { --g_solverCount[umath::to_integral(GetNetworkState().GetType())]; }

void IkSolverComponent::SetIkRigFile(const std::string &RigConfigFile)
{
	m_ikRigFile = RigConfigFile;
	UpdateIkRigFile();
}
const std::string &IkSolverComponent::GetIkRigFile() const { return m_ikRigFile; }
void IkSolverComponent::UpdateIkRigFile()
{
	InitializeSolver(); // Clear Rig
	if(!m_ikRigFile.empty()) {
		auto rigConfig = pragma::ik::RigConfig::load("scripts/ik_rigs/" + m_ikRigFile);
		if(rigConfig)
			AddIkSolverByRig(*rigConfig);
	}
	UpdateIkRig();
}
void IkSolverComponent::InitializeSolver()
{
	m_ikControls.clear();
	m_boneIdToIkBoneId.clear();
	m_ikBoneIdToBoneId.clear();
	m_ikSolver = std::make_unique<pragma::ik::Solver>(100, 10);
	m_pinnedBones.clear();
	UpdateSolverSettings();

	ClearMembers();
	OnMembersChanged();

	BroadcastEvent(EVENT_INITIALIZE_SOLVER);
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
	std::vector<BoneId> ikChain;
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
void IkSolverComponent::AddSkeletalBone(BoneId boneId)
{
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(!mdl)
		return;
	auto &skeleton = mdl->GetSkeleton();
	auto bone = skeleton.GetBone(boneId);
	if(bone.expired())
		return;
	auto &ref = mdl->GetReference();
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId, pose))
		return;
	AddBone(bone.lock()->name, boneId, pose, 1.f, 1.f);
}
void IkSolverComponent::AddDragControl(BoneId boneId, float maxForce, float rigidity) { AddControl(boneId, pragma::ik::RigConfigControl::Type::Drag, maxForce, rigidity); }
void IkSolverComponent::AddStateControl(BoneId boneId, float maxForce, float rigidity) { AddControl(boneId, pragma::ik::RigConfigControl::Type::State, maxForce, rigidity); }
void IkSolverComponent::AddOrientedDragControl(BoneId boneId, float maxForce, float rigidity) { AddControl(boneId, pragma::ik::RigConfigControl::Type::OrientedDrag, maxForce, rigidity); }
size_t IkSolverComponent::GetBoneCount() const { return m_ikSolver->GetBoneCount(); }
pragma::ik::IControl *IkSolverComponent::GetControl(BoneId boneId)
{
	auto it = m_ikControls.find(boneId);
	if(it == m_ikControls.end())
		return nullptr;
	return it->second.get();
}
pragma::ik::Bone *IkSolverComponent::GetBone(BoneId boneId)
{
	auto it = m_boneIdToIkBoneId.find(boneId);
	if(it == m_boneIdToIkBoneId.end())
		return nullptr;
	return m_ikSolver->GetBone(it->second);
}
void IkSolverComponent::SetBoneLocked(BoneId boneId, bool locked)
{
	auto *bone = GetBone(boneId);
	if(!bone)
		return;
	bone->SetPinned(locked);
	auto it = std::find_if(m_pinnedBones.begin(), m_pinnedBones.end(), [boneId](const PinnedBoneInfo &info) { return info.boneId == boneId; });
	if(it != m_pinnedBones.end()) {
		if(!locked)
			m_pinnedBones.erase(it);
		return;
	}
	m_pinnedBones.push_back({});
	auto ikBoneId = GetIkBoneId(boneId);
	assert(ikBoneId.has_value());
	auto &info = m_pinnedBones.back();
	info.boneId = boneId;
	info.ikBoneId = *ikBoneId;
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
		BoneId skeletonBoneIndex;
		HierarchyDepth hierarchyDepth;
	};
	std::vector<BoneInfo> bones;
	auto &rigBones = rigConfig.GetBones();
	bones.reserve(rigBones.size());

	uint32_t rigBoneIdx = 0;
	for(auto &boneData : rigBones) {
		auto boneId = skeleton.LookupBone(boneData->name);
		if(boneId == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Bone {} does not exist in skeleton.", GetEntity().ToString(), boneData->name);
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
		bones.push_back(BoneInfo {rigBoneIdx++, static_cast<BoneId>(boneId), depth});
	}
	// Sort the bones to be in hierarchical order. This is not necessary for the ik solver, but the order is important
	// when the animation is updated.
	std::sort(bones.begin(), bones.end(), [](const BoneInfo &a, const BoneInfo &b) { return a.hierarchyDepth < b.hierarchyDepth; });
	for(auto i = decltype(bones.size()) {0u}; i < bones.size(); ++i) {
		auto &boneInfo = bones[i];
		auto &rigBone = rigBones[boneInfo.rigBoneIndex];
		AddSkeletalBone(boneInfo.skeletonBoneIndex);
		if(rigBone->locked)
			SetBoneLocked(boneInfo.skeletonBoneIndex, true);
	}

	for(auto &controlData : rigConfig.GetControls()) {
		auto boneId = skeleton.LookupBone(controlData->bone);
		if(boneId == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Control bone {} does not exist in skeleton.", GetEntity().ToString(), controlData->bone);
			return false;
		}
		AddControl(boneId, controlData->type, controlData->maxForce, controlData->rigidity);
	}

	for(auto &constraintData : rigConfig.GetConstraints()) {
		auto boneId0 = skeleton.LookupBone(constraintData->bone0);
		auto boneId1 = skeleton.LookupBone(constraintData->bone1);
		if(boneId0 == -1 || boneId1 == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Constraint bone {} or {} does not exist in skeleton.", GetEntity().ToString(), constraintData->bone0, constraintData->bone1);
			return false;
		}

		ConstraintInfo constraintInfo {static_cast<BoneId>(boneId0), static_cast<BoneId>(boneId1)};
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
	return true;
}
std::optional<umath::ScaledTransform> IkSolverComponent::GetReferenceBonePose(BoneId boneId) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return {};
	return mdl->GetReferenceBonePose(boneId);
}
bool IkSolverComponent::GetConstraintBones(BoneId boneId0, BoneId boneId1, pragma::ik::Bone **bone0, pragma::ik::Bone **bone1, umath::ScaledTransform &pose0, umath::ScaledTransform &pose1) const
{
	auto itBone0 = m_boneIdToIkBoneId.find(boneId0);
	auto itBone1 = m_boneIdToIkBoneId.find(boneId1);
	if(itBone0 == m_boneIdToIkBoneId.end() || itBone1 == m_boneIdToIkBoneId.end()) {
		spdlog::debug("Failed to add fixed constraint to ik solver {}: Bone {} or {} do not exist.", GetEntity().ToString(), boneId0, boneId1);
		return false;
	}
	*bone0 = m_ikSolver->GetBone(itBone0->second);
	*bone1 = m_ikSolver->GetBone(itBone1->second);
	if(!*bone0 || !*bone1) {
		spdlog::debug("Failed to add fixed constraint to ik solver {}: Bone {} or {} do not exist in solver.", GetEntity().ToString(), boneId0, boneId1);
		return false;
	}
	auto refPose0 = GetReferenceBonePose(boneId0);
	auto refPose1 = GetReferenceBonePose(boneId1);
	if(!refPose0.has_value() || !refPose1.has_value()) {
		spdlog::debug("Failed to add fixed constraint to ik solver {}: Bone {} or {} do not exist in reference pose.", GetEntity().ToString(), boneId0, boneId1);
		return false;
	}
	pose0 = *refPose0;
	pose1 = *refPose1;
	return true;
}
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
void IkSolverComponent::AddHingeConstraint(const ConstraintInfo &constraintInfo, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation, SignedAxis twistAxis)
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
	switch(twistAxis) {
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

void IkSolverComponent::AddBallSocketConstraint(const ConstraintInfo &constraintInfo, const EulerAngles &minLimits, const EulerAngles &maxLimits, SignedAxis twistAxis)
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
	for(uint8_t i = 0; i < 3; ++i)
		clamp_angles(effectiveMinLimits[i], effectiveMaxLimits[i]);

	auto &rotBone0 = refPose0.GetRotation();
	auto &rotBone1 = refPose1.GetRotation();

	// If the twist axis is NOT the Z axis, we'll have to rotate
	// the main axis around a bit and adjust the limits accordingly.
	auto twistRotOffset = Model::GetTwistAxisRotationOffset(twistAxis);
	uquat::inverse(twistRotOffset);
	{
		switch(twistAxis) {
		case pragma::SignedAxis::X:
		case pragma::SignedAxis::NegX:
			umath::swap(effectiveMinLimits.r, effectiveMinLimits.y);
			umath::swap(effectiveMaxLimits.r, effectiveMaxLimits.y);
			break;
		case pragma::SignedAxis::Y:
		case pragma::SignedAxis::NegY:
			umath::swap(effectiveMinLimits.p, effectiveMinLimits.r);
			umath::swap(effectiveMaxLimits.p, effectiveMaxLimits.r);

			umath::swap(effectiveMinLimits.y, effectiveMaxLimits.y);
			effectiveMinLimits.y *= -1.f;
			effectiveMaxLimits.y *= -1.f;

			//umath::swap(effectiveMinLimits.p, effectiveMinLimits.y);
			break;
		case pragma::SignedAxis::Z:
		case pragma::SignedAxis::NegZ:
			umath::swap(effectiveMinLimits.p, effectiveMinLimits.y);
			umath::swap(effectiveMaxLimits.p, effectiveMaxLimits.y);
			break;
		}
	}

	auto refRot1 = rotBone1 * twistRotOffset; //uquat::create(ang);

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

	// This is not quite correct and can cause issues with certain configurations.
	auto rotBone1WithOffset = refRot1 * uquat::create(EulerAngles(-(effectiveMaxLimits.p + effectiveMinLimits.p), effectiveMaxLimits.y + effectiveMinLimits.y, 0.f)); //-(effectiveMaxLimits.r + effectiveMinLimits.r)));

	auto axisB = uquat::forward(refRot1);
	auto span = effectiveMaxLimits.y - effectiveMinLimits.y;
	if(span >= 0.f && span < 179.99f) {
		if(!useEllipseSwingLimit) {
			auto &ellipseSwingLimit = m_ikSolver->AddSwingLimit(*bone0, *bone1, uquat::forward(rotBone1WithOffset), axisB, umath::deg_to_rad(effectiveMaxLimits.y - effectiveMinLimits.y));
			init_joint(constraintInfo, ellipseSwingLimit);
		}
		else {
			auto &ellipseSwingLimit = m_ikSolver->AddEllipseSwingLimit(*bone0, *bone1, uquat::forward(rotBone1WithOffset), axisB, umath::deg_to_rad(effectiveMaxLimits.y - effectiveMinLimits.y), umath::deg_to_rad(effectiveMaxLimits.p - effectiveMinLimits.p));
			init_joint(constraintInfo, ellipseSwingLimit);
		}
	}

	auto twistLimitVal = effectiveMaxLimits.r - effectiveMinLimits.r;
	if(twistLimitVal >= 0.f && twistLimitVal < 179.99f) {
		auto &twistLimit = m_ikSolver->AddTwistLimit(*bone0, *bone1, uquat::forward(rotBone1WithOffset), axisB, umath::deg_to_rad(twistLimitVal));
		init_joint(constraintInfo, twistLimit);
	}
}
bool IkSolverComponent::UpdateIkRig()
{
	udm::LinkedPropertyWrapper prop {*m_ikRig};
	auto rig = pragma::ik::RigConfig::load_from_udm_data(prop);
	if(rig)
		AddIkSolverByRig(*rig);
	OnMembersChanged();
	return true;
}
void IkSolverComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void IkSolverComponent::OnEntitySpawn()
{
	SetTickPolicy(pragma::TickPolicy::Always);
	InitializeSolver();
}
std::optional<IkSolverComponent::IkBoneId> IkSolverComponent::GetIkBoneId(BoneId boneId) const
{
	auto it = m_boneIdToIkBoneId.find(boneId);
	if(it == m_boneIdToIkBoneId.end())
		return {};
	return it->second;
}
std::optional<std::string> IkSolverComponent::GetControlBoneName(const std::string &propPath)
{
	auto path = util::Path::CreatePath(propPath);
	size_t nextOffset = 0;
	if(path.GetComponent(0, &nextOffset) != "control")
		return {};
	auto boneName = path.GetComponent(nextOffset);
	if(boneName.empty())
		return {};
	return std::string {boneName};
}
std::optional<BoneId> IkSolverComponent::GetControlBoneId(const std::string &propPath)
{
	auto boneName = GetControlBoneName(propPath);
	if(!boneName)
		return {};
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return {};
	auto &skeleton = mdl->GetSkeleton();
	auto boneId = skeleton.LookupBone(*boneName);
	if(boneId == -1)
		return {};
	return boneId;
}
std::optional<BoneId> IkSolverComponent::GetSkeletalBoneId(IkBoneId boneId) const
{
	auto it = m_ikBoneIdToBoneId.find(boneId);
	if(it == m_ikBoneIdToBoneId.end())
		return {};
	return it->second;
}
pragma::ik::Bone *IkSolverComponent::GetIkBone(BoneId boneId)
{
	auto rigConfigBoneId = GetIkBoneId(boneId);
	if(!rigConfigBoneId.has_value())
		return nullptr;
	return m_ikSolver->GetBone(*rigConfigBoneId);
}
void IkSolverComponent::AddControl(BoneId boneId, pragma::ik::RigConfigControl::Type type, float maxForce, float rigidity)
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl) {
		spdlog::debug("Failed to add control to ik solver {}: Entity has no model.", GetEntity().ToString());
		return;
	}
	auto &skeleton = mdl->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(!bone) {
		spdlog::debug("Failed to add control to ik solver {}: Control bone {} does not exist in skeleton.", GetEntity().ToString(), boneId);
		return;
	}
	auto rigConfigBoneId = GetIkBoneId(boneId);
	if(rigConfigBoneId.has_value() == false) {
		spdlog::debug("Failed to add control to ik solver {}: Control bone {} does not exist in ik rig.", GetEntity().ToString(), boneId);
		return;
	}
	auto *rigConfigBone = m_ikSolver->GetBone(*rigConfigBoneId);
	if(!rigConfigBone) {
		spdlog::debug("Failed to add control to ik solver {}: Control bone {} does not exist in ik rig.", GetEntity().ToString(), boneId);
		return;
	}
	if(m_ikSolver->FindControl(*rigConfigBone) != nullptr)
		return;
	pragma::ik::IControl *control = nullptr;
	switch(type) {
	case pragma::ik::RigConfigControl::Type::State:
		{
			auto &stateControl = m_ikSolver->AddStateControl(*rigConfigBone);
			stateControl.SetTargetPosition(rigConfigBone->GetPos());
			stateControl.SetTargetOrientation(rigConfigBone->GetRot());
			control = &stateControl;
			break;
		}
	case pragma::ik::RigConfigControl::Type::Drag:
		{
			auto &dragControl = m_ikSolver->AddDragControl(*rigConfigBone);
			dragControl.SetTargetPosition(rigConfigBone->GetPos());
			control = &dragControl;
			break;
		}
	case pragma::ik::RigConfigControl::Type::OrientedDrag:
		{
			auto &dragControl = m_ikSolver->AddOrientedDragControl(*rigConfigBone);
			dragControl.SetTargetPosition(rigConfigBone->GetPos());
			dragControl.SetTargetOrientation(rigConfigBone->GetRot());
			control = &dragControl;
			break;
		}
	}
	static_assert(umath::to_integral(pragma::ik::RigConfigControl::Type::Count) == 3u);

	control->SetMaxForce((maxForce < 0.f) ? std::numeric_limits<float>::max() : maxForce);
	control->SetRigidity(rigidity);
	auto &name = bone->name;
	using TComponent = IkSolverComponent;
	auto defGetSet = [this, &bone, rigConfigBone, &name, boneId](auto &ctrl) {
		using TControl = std::remove_reference_t<decltype(ctrl)>;

		auto posePropName = "control/" + name + "/pose";
		auto posPropName = "control/" + name + "/position";
		auto rotPropName = "control/" + name + "/rotation";

		auto coordMetaData = std::make_shared<ents::CoordinateTypeMetaData>();
		coordMetaData->space = umath::CoordinateSpace::Object;

		constexpr auto hasRotation = std::is_same_v<TControl, pragma::ik::OrientedDragControl> || std::is_same_v<TControl, pragma::ik::StateControl>;

		std::shared_ptr<ents::PoseComponentTypeMetaData> compMetaData = nullptr;
		if constexpr(hasRotation) {
			compMetaData = std::make_shared<ents::PoseComponentTypeMetaData>();
			compMetaData->poseProperty = posePropName;
		}

		auto memberInfoPos = pragma::ComponentMemberInfo::CreateDummy();
		memberInfoPos.SetName(posPropName);
		memberInfoPos.type = ents::EntityMemberType::Vector3;
		memberInfoPos.userIndex = boneId;
		memberInfoPos.AddTypeMetaData(coordMetaData);
		if(compMetaData)
			memberInfoPos.AddTypeMetaData(compMetaData);
		memberInfoPos.SetFlag(pragma::ComponentMemberFlags::ObjectSpace);
		using TValue = Vector3;
		memberInfoPos.SetGetterFunction<TComponent, TValue, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, TValue &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
			auto it = component.m_ikControls.find(memberInfo.userIndex);
			if(it == component.m_ikControls.end()) {
				outValue = {};
				return;
			}
			outValue = static_cast<TControl *>(it->second.get())->GetTargetPosition();
		})>();
		memberInfoPos.SetSetterFunction<TComponent, TValue, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, const TValue &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
			auto it = component.m_ikControls.find(memberInfo.userIndex);
			if(it == component.m_ikControls.end())
				return;
			static_cast<TControl *>(it->second.get())->SetTargetPosition(value);
			component.m_updateRequired = true;
		})>();
		RegisterMember(std::move(memberInfoPos));
		ctrl.SetTargetPosition(rigConfigBone->GetPos());

		if constexpr(hasRotation) {
			auto memberInfoRot = pragma::ComponentMemberInfo::CreateDummy();
			memberInfoRot.SetName(rotPropName);
			memberInfoRot.type = ents::EntityMemberType::Quaternion;
			memberInfoRot.userIndex = boneId;
			memberInfoRot.AddTypeMetaData(coordMetaData);
			memberInfoRot.AddTypeMetaData(compMetaData);
			memberInfoRot.SetFlag(pragma::ComponentMemberFlags::ObjectSpace);
			using TValue = Quat;
			memberInfoRot.SetGetterFunction<TComponent, TValue, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, TValue &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
				auto it = component.m_ikControls.find(memberInfo.userIndex);
				if(it == component.m_ikControls.end()) {
					outValue = {};
					return;
				}
				outValue = static_cast<TControl *>(it->second.get())->GetTargetOrientation();
			})>();
			memberInfoRot.SetSetterFunction<TComponent, TValue, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, const TValue &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
				auto it = component.m_ikControls.find(memberInfo.userIndex);
				if(it == component.m_ikControls.end())
					return;
				static_cast<TControl *>(it->second.get())->SetTargetOrientation(value);
				component.m_updateRequired = true;
			})>();
			RegisterMember(std::move(memberInfoRot));
			ctrl.SetTargetOrientation(rigConfigBone->GetRot());

			auto poseMetaData = std::make_shared<ents::PoseTypeMetaData>();
			poseMetaData->posProperty = posPropName;
			poseMetaData->rotProperty = rotPropName;

			auto memberInfoPose = pragma::ComponentMemberInfo::CreateDummy();
			memberInfoPose.SetName(posePropName);
			memberInfoPose.type = ents::EntityMemberType::Transform;
			memberInfoPose.userIndex = boneId;
			memberInfoPose.AddTypeMetaData(coordMetaData);
			memberInfoPose.AddTypeMetaData(poseMetaData);
			memberInfoPose.SetFlag(pragma::ComponentMemberFlags::ObjectSpace);
			using TValuePose = umath::Transform;
			memberInfoPose.SetGetterFunction<TComponent, TValuePose, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, TValuePose &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValuePose &outValue) {
				auto it = component.m_ikControls.find(memberInfo.userIndex);
				if(it == component.m_ikControls.end()) {
					outValue = {};
					return;
				}
				auto *ctrl = static_cast<TControl *>(it->second.get());
				outValue = {ctrl->GetTargetPosition(), ctrl->GetTargetOrientation()};
			})>();
			memberInfoPose.SetSetterFunction<TComponent, TValuePose, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, const TValuePose &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValuePose &value) {
				auto it = component.m_ikControls.find(memberInfo.userIndex);
				if(it == component.m_ikControls.end())
					return;
				auto *ctrl = static_cast<TControl *>(it->second.get());
				ctrl->SetTargetPosition(value.GetOrigin());
				ctrl->SetTargetOrientation(value.GetRotation());
				component.m_updateRequired = true;
			})>();
			RegisterMember(std::move(memberInfoPose));
		}
		//}
	};
	switch(type) {
	case pragma::ik::RigConfigControl::Type::State:
		defGetSet(static_cast<pragma::ik::StateControl &>(*control));
		break;
	case pragma::ik::RigConfigControl::Type::Drag:
		defGetSet(static_cast<pragma::ik::DragControl &>(*control));
		break;
	case pragma::ik::RigConfigControl::Type::OrientedDrag:
		defGetSet(static_cast<pragma::ik::OrientedDragControl &>(*control));
		break;
	}
	static_assert(umath::to_integral(pragma::ik::RigConfigControl::Type::Count) == 3u);

#if 0
	auto memberInfoLocked = pragma::ComponentMemberInfo::CreateDummy();
	memberInfoLocked.SetName("control/" + name + "/locked");
	memberInfoLocked.type = ents::EntityMemberType::Boolean;
	memberInfoLocked.userIndex = boneId;
	using TValue = bool;
	memberInfoLocked.SetGetterFunction<TComponent, TValue, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, TValue &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, TValue &outValue) {
		auto it = component.m_boneIdToIkBoneId.find(memberInfo.userIndex);
		if(it == component.m_boneIdToIkBoneId.end()) {
			outValue = false;
			return;
		}
		auto *bone = component.m_ikSolver->GetBone(it->second);
		if(!bone) {
			outValue = false;
			return;
		}
		outValue = bone->IsPinned();
	})>();
	memberInfoLocked.SetSetterFunction<TComponent, TValue, static_cast<void (*)(const pragma::ComponentMemberInfo &, TComponent &, const TValue &)>([](const pragma::ComponentMemberInfo &memberInfo, TComponent &component, const TValue &value) {
		auto it = component.m_boneIdToIkBoneId.find(memberInfo.userIndex);
		if(it == component.m_boneIdToIkBoneId.end())
			return;
		auto *bone = component.m_ikSolver->GetBone(it->second);
		if(!bone)
			return;
		bone->SetPinned(value);
	})>();
	RegisterMember(std::move(memberInfoLocked));
#endif

	// TODO: Position weight and rotation weight

	m_ikControls[boneId] = m_ikSolver->FindControlPtr(*rigConfigBone);
}
const ComponentMemberInfo *IkSolverComponent::GetMemberInfo(ComponentMemberIndex idx) const
{
	auto numStatic = GetStaticMemberCount();
	if(idx < numStatic)
		return BaseEntityComponent::GetMemberInfo(idx);
	return DynamicMemberRegister::GetMemberInfo(idx);
}

std::optional<ComponentMemberIndex> IkSolverComponent::DoGetMemberIndex(const std::string &name) const
{
	auto idx = BaseEntityComponent::DoGetMemberIndex(name);
	if(idx.has_value())
		return idx;
	idx = DynamicMemberRegister::GetMemberIndex(name);
	if(idx.has_value())
		return *idx; // +GetStaticMemberCount();
	return std::optional<ComponentMemberIndex> {};
}
pragma::ik::Bone *IkSolverComponent::AddBone(const std::string &boneName, BoneId boneId, const umath::Transform &pose, float radius, float length)
{
	auto rigConfigBone = GetIkBone(boneId);
	if(rigConfigBone)
		return rigConfigBone;
	IkBoneId rigConfigBoneId;
	rigConfigBone = &m_ikSolver->AddBone(pose.GetOrigin(), pose.GetRotation(), radius, length, &rigConfigBoneId);
	rigConfigBone->SetName(boneName);
	m_boneIdToIkBoneId[boneId] = rigConfigBoneId;
	m_ikBoneIdToBoneId[rigConfigBoneId] = boneId;
	return rigConfigBone;
}
void IkSolverComponent::SetResetSolver(bool resetSolver) { m_resetIkPose = resetSolver; }
bool IkSolverComponent::ShouldResetSolver() const { return m_resetIkPose; }
void IkSolverComponent::ResetIkRig() { InitializeSolver(); }
const std::shared_ptr<pragma::ik::Solver> &IkSolverComponent::GetIkSolver() const { return m_ikSolver; }

void IkSolverComponent::Solve()
{
	if(!m_updateRequired) {
		auto animC = GetEntity().GetAnimatedComponent();
		for(auto &info : m_pinnedBones) {
			Vector3 pos;
			Quat rot;
			Vector3 scale;
			if(animC.valid() && animC->GetLocalBonePosition(info.boneId, pos, rot, &scale)) {
				if(uvec::distance_sqr(pos, info.oldPose.GetOrigin()) > 0.00001f || uquat::dot_product(rot, info.oldPose.GetRotation()) < 0.99999f || uvec::distance_sqr(scale, info.oldPose.GetScale()) > 0.00001f) {
					m_updateRequired = true;
				}
			}
		}
	}

	if(!m_updateRequired) {
		InvokeEventCallbacks(EVENT_ON_IK_UPDATED);
		return;
	}
	m_updateRequired = false;

	{
		auto animC = GetEntity().GetAnimatedComponent();
		for(auto &info : m_pinnedBones) {
			auto animC = GetEntity().GetAnimatedComponent(); // TODO: This is not very fast...
			Vector3 pos;
			Quat rot;
			Vector3 scale;
			if(animC.valid() && animC->GetLocalBonePosition(info.boneId, pos, rot, &scale)) {
				auto *ikBone = m_ikSolver->GetBone(info.ikBoneId);
				if(ikBone) {
					ikBone->SetPos(pos);
					ikBone->SetRot(rot);
				}
				info.oldPose = {pos, rot, scale};
			}
		}
	}

	if(m_resetIkPose)
		ResetIkBones();
	m_ikSolver->Solve();
	InvokeEventCallbacks(EVENT_ON_IK_UPDATED);
}
void IkSolverComponent::ResetIkBones()
{
	auto numBones = m_ikSolver->GetBoneCount();
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
		auto *bone = m_ikSolver->GetBone(i);
		if(!bone || bone->IsPinned())
			continue; // Pinned bones are handled via forward kinematics
		auto it = m_ikBoneIdToBoneId.find(i);
		if(it == m_ikBoneIdToBoneId.end())
			continue;
		auto boneId = it->second;
		auto pose = GetReferenceBonePose(boneId);
		if(!pose.has_value())
			continue;
		bone->SetPos(pose->GetOrigin());
		bone->SetRot(pose->GetRotation());
	}
}
