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
#include "pragma/entities/entity_component_system_t.hpp"
#include "pragma/model/model.h"
#include "pragma/logging.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>

using namespace pragma;

ComponentEventId IkSolverComponent::EVENT_INITIALIZE_SOLVER = pragma::INVALID_COMPONENT_ID;
ComponentEventId IkSolverComponent::EVENT_UPDATE_IK = pragma::INVALID_COMPONENT_ID;
void IkSolverComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_INITIALIZE_SOLVER = registerEvent("INITIALIZE_SOLVER", ComponentEventInfo::Type::Broadcast);
	EVENT_UPDATE_IK = registerEvent("UPDATE_IK", ComponentEventInfo::Type::Explicit);
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
	UpdateSolverSettings();

	ClearMembers();
	OnMembersChanged();

	BroadcastEvent(EVENT_INITIALIZE_SOLVER);
}
bool IkSolverComponent::AddIkSolverByChain(const std::string &boneName, uint32_t chainLength)
{
	constexpr uint32_t minChainLength = 2;
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
	auto effectiveChainLength = chainLength + 1;
	std::vector<BoneId> ikChain;
	ikChain.reserve(effectiveChainLength);
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId == -1) {
		spdlog::debug("Failed to add ik chain to ik solver {} with boneName={} and chainLength={}: Bone not found in skeleton.", GetEntity().ToString(), boneName, chainLength);
		return false;
	}

	auto bone = skeleton.GetBone(boneId).lock();
	for(auto i = decltype(effectiveChainLength) {0}; i < effectiveChainLength; ++i) {
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
		if(i == ikChain.size() - 1)
			rig.AddControl(skeleton.GetBone(ikChain[i]).lock()->name, pragma::ik::RigConfigControl::Type::State);
		else
			rig.AddControl(skeleton.GetBone(ikChain[i]).lock()->name, pragma::ik::RigConfigControl::Type::Drag);
	}

	// Add generic ballsocket constraints with no twist
	for(auto i = decltype(ikChain.size()) {1u}; i < ikChain.size(); ++i) {
		// We need to allow some minor twisting to avoid instability
		rig.AddBallSocketConstraint(skeleton.GetBone(ikChain[i - 1]).lock()->name, skeleton.GetBone(ikChain[i]).lock()->name, EulerAngles(-90, -90, -0.5), EulerAngles(90, 90, 0.5));
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
void IkSolverComponent::AddDragControl(BoneId boneId) { AddControl(boneId, true, false); }
void IkSolverComponent::AddStateControl(BoneId boneId) { AddControl(boneId, true, true); }
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
}
bool IkSolverComponent::AddIkSolverByRig(const pragma::ik::RigConfig &rigConfig)
{
	auto mdl = GetEntity().GetModel();
	if(!mdl) {
		spdlog::debug("Failed to add ik rig to ik solver {}: Entity has no model.", GetEntity().ToString());
		return false;
	}
	auto &skeleton = mdl->GetSkeleton();
	for(auto &boneData : rigConfig.GetBones()) {
		auto boneId = skeleton.LookupBone(boneData->name);
		if(boneId == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Bone {} does not exist in skeleton.", GetEntity().ToString(), boneData->name);
			return false;
		}
		AddSkeletalBone(boneId);
		if(boneData->locked)
			SetBoneLocked(boneId, true);
	}

	for(auto &controlData : rigConfig.GetControls()) {
		auto boneId = skeleton.LookupBone(controlData->bone);
		if(boneId == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Control bone {} does not exist in skeleton.", GetEntity().ToString(), controlData->bone);
			return false;
		}
		switch(controlData->type) {
		case pragma::ik::RigConfigControl::Type::Drag:
			AddDragControl(boneId);
			break;
		case pragma::ik::RigConfigControl::Type::State:
			AddStateControl(boneId);
			break;
		}
	}

	for(auto &constraintData : rigConfig.GetConstraints()) {
		auto boneId0 = skeleton.LookupBone(constraintData->bone0);
		auto boneId1 = skeleton.LookupBone(constraintData->bone1);
		if(boneId0 == -1 || boneId1 == -1) {
			spdlog::debug("Failed to add ik rig to ik solver {}: Constraint bone {} or {} does not exist in skeleton.", GetEntity().ToString(), constraintData->bone0, constraintData->bone1);
			return false;
		}
		switch(constraintData->type) {
		case pragma::ik::RigConfigConstraint::Type::Fixed:
			AddFixedConstraint(boneId0, boneId1);
			break;
		case pragma::ik::RigConfigConstraint::Type::Hinge:
			AddHingeConstraint(boneId0, boneId1, constraintData->minLimits.p, constraintData->maxLimits.p, constraintData->offsetPose.GetRotation(), constraintData->axis);
			break;
		case pragma::ik::RigConfigConstraint::Type::BallSocket:
			AddBallSocketConstraint(boneId0, boneId1, constraintData->minLimits, constraintData->maxLimits, constraintData->axis);
			break;
		}
	}
	return true;
}
std::optional<umath::ScaledTransform> IkSolverComponent::GetReferenceBonePose(Model &model, BoneId boneId)
{
	auto &ref = model.GetReference();
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId, pose))
		return {};
	return pose;
}
std::optional<umath::ScaledTransform> IkSolverComponent::GetReferenceBonePose(BoneId boneId) const
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return {};
	return GetReferenceBonePose(*mdl, boneId);
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
void IkSolverComponent::AddFixedConstraint(BoneId boneId0, BoneId boneId1)
{
	pragma::ik::Bone *bone0, *bone1;
	umath::ScaledTransform refPose0, refPose1;
	if(!GetConstraintBones(boneId0, boneId1, &bone0, &bone1, refPose0, refPose1))
		return;
	auto &rotBone0 = refPose0.GetRotation();
	auto &rotBone1 = refPose1.GetRotation();

	// Lock distance and rotation to the parent
	m_ikSolver->AddBallSocketJoint(*bone0, *bone1, bone1->GetPos());

	// Lock the angles
	auto &joint = m_ikSolver->AddAngularJoint(*bone0, *bone1);
	joint.SetRigidity(1'000.f);

	// Lock swing limit to 0
	//self.m_solver:AddSwingLimit(bone0,bone1,self:GetDirectionFromBoneParent(boneId1),self:GetDirectionFromBoneParent(boneId1),math.rad(5)):SetRigidity(1000)

	// Restrict twist
	//self.m_solver:AddTwistLimit(bone0,bone1,rotBone1:GetForward(),rotBone1:GetForward(),math.rad(5)):SetRigidity(1000)
}
void IkSolverComponent::AddHingeConstraint(BoneId boneId0, BoneId boneId1, umath::Degree minAngle, umath::Degree maxAngle, const Quat &offsetRotation, Axis twistAxis)
{
	pragma::ik::Bone *bone0, *bone1;
	umath::ScaledTransform refPose0, refPose1;
	if(!GetConstraintBones(boneId0, boneId1, &bone0, &bone1, refPose0, refPose1))
		return;
	auto &rotBone0 = refPose0.GetRotation();
	auto &rotBone1 = refPose1.GetRotation();

	auto twistRotOffset = uquat::identity();
	{
		// If the twist axis is NOT the X axis, we'll have to rotate
		// the main axis around a bit and adjust the limits accordingly.
		switch(twistAxis) {
		case pragma::Axis::X:
			break;
		case pragma::Axis::Y:
			twistRotOffset = uquat::create(EulerAngles(0.0, 0.f, 90.f));
			break;
		case pragma::Axis::Z:
			twistRotOffset = uquat::create(EulerAngles(0.0, 90.f, 0.f));
			umath::swap(minAngle, maxAngle);
			minAngle *= -1.f;
			maxAngle *= -1.f;
			break;
		}
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
	m_ikSolver->AddBallSocketJoint(*bone0, *bone1, bone1->GetPos());

	// Revolute joint to lock rotation to a single axis
	m_ikSolver->AddRevoluteJoint(*bone0, *bone1, uquat::right(rotBone1));

	// Apply the swing limit
	auto &swingLimit = m_ikSolver->AddSwingLimit(*bone0, *bone1, uquat::up(rotBone1WithOffset), uquat::up(rotBone1), umath::deg_to_rad(maxAngle - minAngle));
	swingLimit.SetRigidity(1'000);

	/*
	local rotBone0 = self:GetReferenceBonePose(boneId0):GetRotation()
	local rotBone1 = self:GetReferenceBonePose(boneId1):GetRotation()

	-- TODO: This does not belong here
	rotBone0 = self:GetEntity():GetPose() *rotBone0
	rotBone1 = self:GetEntity():GetPose() *rotBone1

	-- The IK system only allows us to specify a general swing limit (in any direction). Since we want to be able to specify it in each
	-- direction independently, we have to shift the rotation axes accordingly.
	local rotBone1WithOffset = rotBone1 *EulerAngles(0,-(max +min),0):ToQuaternion()

	local bone0 = self.m_solver:GetBone(self.m_boneIdToRigConfigBoneId[boneId0])
	local bone1 = self.m_solver:GetBone(self.m_boneIdToRigConfigBoneId[boneId1])

	-- BallSocket is required to ensure the distance and rotation to the parent is locked
	self.m_solver:AddBallSocketJoint(bone0,bone1,bone1:GetPos())

	-- Revolute joint to lock rotation to a single axis
	self.m_solver:AddRevoluteJoint(bone0,bone1,rotBone0:GetUp())

	-- Apply the swing limit
	self.m_solver:AddSwingLimit(bone0,bone1,rotBone1WithOffset:GetForward(),rotBone1:GetForward(),math.rad(max -min)):SetRigidity(1000)
*/
}

std::optional<pragma::Axis> IkSolverComponent::FindTwistAxis(Model &mdl, BoneId boneId)
{
	auto refPose = IkSolverComponent::GetReferenceBonePose(mdl, boneId);
	if(!refPose)
		return {};

	auto bone = mdl.GetSkeleton().GetBone(boneId).lock();
	std::vector<Vector3> normalList;
	normalList.reserve(bone->children.size());
	for(auto &pair : bone->children) {
		auto pose = IkSolverComponent::GetReferenceBonePose(mdl, pair.first);
		if(pose) {
			auto normal = pose->GetOrigin() - refPose->GetOrigin();
			uvec::normalize(&normal);
			normalList.push_back(normal);
		}
	}
	Vector3 norm = uvec::FORWARD;
	if(!normalList.empty())
		norm = uvec::calc_average(normalList);
	else {
		if(!bone->parent.expired()) {
			auto refPoseParent = IkSolverComponent::GetReferenceBonePose(mdl, bone->parent.lock()->ID);
			if(refPoseParent)
				norm = (refPose->GetOrigin() - refPoseParent->GetOrigin());
		}
	}

	auto &rotBone1 = refPose->GetRotation();
	auto dirFromBone0ToBone1 = norm;
	//
	uvec::normalize(&dirFromBone0ToBone1);
	auto forward = uquat::forward(rotBone1);
	auto right = uquat::right(rotBone1);
	auto up = uquat::up(rotBone1);
	auto df = uvec::dot(dirFromBone0ToBone1, forward);
	auto dr = uvec::dot(dirFromBone0ToBone1, right);
	auto du = uvec::dot(dirFromBone0ToBone1, up);
	auto dfa = umath::abs(df);
	auto dra = umath::abs(dr);
	auto dua = umath::abs(du);
	if(dfa >= umath::max(dra, dua))
		return pragma::Axis::Z; // Forward
	else if(dra >= umath::max(dfa, dua))
		return pragma::Axis::X; // Right
	return pragma::Axis::Y;     // Up
}

void IkSolverComponent::AddBallSocketConstraint(BoneId boneId0, BoneId boneId1, const EulerAngles &minLimits, const EulerAngles &maxLimits, Axis twistAxis)
{
	pragma::ik::Bone *bone0, *bone1;
	umath::ScaledTransform refPose0, refPose1;
	if(!GetConstraintBones(boneId0, boneId1, &bone0, &bone1, refPose0, refPose1))
		return;
	// BallSocket is required to ensure the distance and rotation to the parent is locked
	m_ikSolver->AddBallSocketJoint(*bone0, *bone1, bone1->GetPos());

	auto effectiveMinLimits = minLimits * 0.5f;
	auto effectiveMaxLimits = maxLimits * 0.5f;
	for(uint8_t i = 0; i < 3; ++i)
		clamp_angles(effectiveMinLimits[i], effectiveMaxLimits[i]);

	auto &rotBone0 = refPose0.GetRotation();
	auto &rotBone1 = refPose1.GetRotation();

	Quat twistRotOffset = uquat::identity();
	{
		// If the twist axis is NOT the Z axis, we'll have to rotate
		// the main axis around a bit and adjust the limits accordingly.
		switch(twistAxis) {
		case pragma::Axis::X:
			twistRotOffset = uquat::create(EulerAngles(0.0, -90.f, 0.f));
			umath::swap(effectiveMinLimits.r, effectiveMinLimits.y);
			umath::swap(effectiveMaxLimits.r, effectiveMaxLimits.y);
			break;
		case pragma::Axis::Y:
			twistRotOffset = uquat::create(EulerAngles(-90.0, 0.f, 0.f));
			umath::swap(effectiveMinLimits.p, effectiveMinLimits.r);
			umath::swap(effectiveMaxLimits.p, effectiveMaxLimits.r);

			umath::swap(effectiveMinLimits.y, effectiveMaxLimits.y);
			effectiveMinLimits.y *= -1.f;
			effectiveMaxLimits.y *= -1.f;
			break;
		case pragma::Axis::Z:
			break;
		}
	}

	// The IK system only allows us to specify a general swing limit (in any direction). Since we want to be able to specify it in each
	// direction independently, we have to shift the rotation axes accordingly.
	auto rotBone1WithOffset = rotBone1; // * uquat::create(EulerAngles(effectiveMaxLimits.p + effectiveMinLimits.p, effectiveMaxLimits.y + effectiveMinLimits.y, 0.f)); //-(effectiveMaxLimits.r + effectiveMinLimits.r)));
	                                    //uquat::rotate_y(rotBone1WithOffset, umath::deg_to_rad(effectiveMaxLimits.y + effectiveMinLimits.y));
	                                    //uquat::rotate_x(rotBone1WithOffset, umath::deg_to_rad(effectiveMaxLimits.p + effectiveMinLimits.p));


	{
		auto refRot1 = rotBone1 * twistRotOffset; //uquat::create(ang);

		auto maxSpan = umath::max(effectiveMaxLimits.p - effectiveMinLimits.p, effectiveMaxLimits.y - effectiveMinLimits.y);
		auto pMid = (effectiveMaxLimits.p + effectiveMinLimits.p) * 0.5f;
		effectiveMaxLimits.p = pMid + maxSpan * 0.5f;
		effectiveMinLimits.p = pMid - maxSpan * 0.5f;
		auto yMid = (effectiveMaxLimits.y + effectiveMinLimits.y) * 0.5f;
		effectiveMaxLimits.y = yMid + maxSpan * 0.5f;
		effectiveMinLimits.y = yMid - maxSpan * 0.5f;

		// This is not quite correct and can cause issues with certain configurations.
		auto rotBone1WithOffset = refRot1 * uquat::create(EulerAngles(effectiveMaxLimits.p + effectiveMinLimits.p, effectiveMaxLimits.y + effectiveMinLimits.y, 0.f)); //-(effectiveMaxLimits.r + effectiveMinLimits.r)));

		auto axisB = uquat::forward(refRot1);
		auto &ellipseSwingLimit = m_ikSolver->AddSwingLimit(*bone0, *bone1, uquat::forward(rotBone1WithOffset), axisB, umath::deg_to_rad(effectiveMaxLimits.y - effectiveMinLimits.y));
		ellipseSwingLimit.SetRigidity(1'000);

		auto &twistLimit = m_ikSolver->AddTwistLimit(*bone0, *bone1, uquat::forward(rotBone1WithOffset), uquat::up(rotBone1WithOffset), umath::deg_to_rad(1.f)); //effectiveMaxLimits.r - effectiveMinLimits.r));
		twistLimit.SetRigidity(1'000);
	}

	// Revolute joint to lock rotation to a single axis
	//self.m_solver:AddRevoluteJoint(bone0,bone1,rotBone0:GetUp()) -- Test

	// Apply the swing limit
	//self.m_solver:AddSwingLimit(bone0,bone1,rotBone1WithOffset:GetForward(),rotBone1:GetForward(),math.rad(effectiveMaxLimits.y -effectiveMinLimits.y)):SetRigidity(16)
	//self.m_solver:AddSwingLimit(bone0,bone1,rotBone1:GetForward(),rotBone1:GetForward(),math.rad(45)):SetRigidity(16)
	GetEntity().GetNetworkState()->GetGameState()->DrawLine((GetReferenceBonePose(boneId1))->GetOrigin(), (GetReferenceBonePose(boneId1))->GetOrigin() + uquat::up(rotBone1) * -200.f, Color::Red, 12.f);
	auto &ellipseSwingLimit
	  = m_ikSolver->AddEllipseSwingLimit(*bone0, *bone1, uquat::forward(rotBone1WithOffset), uquat::forward(rotBone1), uquat::up(rotBone1WithOffset), umath::deg_to_rad(effectiveMaxLimits.p - effectiveMinLimits.p), umath::deg_to_rad(effectiveMaxLimits.y - effectiveMinLimits.y));
	ellipseSwingLimit.SetRigidity(1'000);

	auto &twistLimit = m_ikSolver->AddTwistLimit(*bone0, *bone1, uquat::forward(rotBone1WithOffset), uquat::forward(rotBone1), umath::deg_to_rad(effectiveMaxLimits.r - effectiveMinLimits.r));
	twistLimit.SetRigidity(1'000);

	//self:GetRigConfig():AddArray("constraints",udm.TYPE_ELEMENT)

	/*local pose0 = self:GetReferenceBonePose(boneId0)
	local pose1 = self:GetReferenceBonePose(boneId1)
	local rot = pose0:GetRotation()
	local angOffset = (effectiveMaxLimits -effectiveMinLimits) /2.0
	rot = rot *angOffset:ToQuaternion()

	local swingSpan = math.max(
		effectiveMaxLimits.p -effectiveMinLimits.p,
		effectiveMaxLimits.y -effectiveMinLimits.y
	)
	local twistSpan = effectiveMaxLimits.r -effectiveMinLimits.r

	local bone0 = self.m_solver:GetBone(self.m_boneIdToRigConfigBoneId[boneId0])
	local bone1 = self.m_solver:GetBone(self.m_boneIdToRigConfigBoneId[boneId1])
	self.m_solver:AddBallSocketJoint(bone0,bone1,pose0:GetOrigin())

	local axis = rot:GetForward()
	--self.m_solver:AddSwingLimit(bone0,bone1,axis,(pose1:GetOrigin() -pose0:GetOrigin()):GetNormal(),math.rad(10)):SetRigidity(1000)
*/
	// TODO: Add twist limit
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
void IkSolverComponent::AddControl(BoneId boneId, bool translation, bool rotation)
{
	assert((translation && rotation) || (translation && !rotation));
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
	pragma::ik::RigConfigControl::Type type;
	if(rotation) {
		auto &stateControl = m_ikSolver->AddStateControl(*rigConfigBone);
		stateControl.SetTargetPosition(rigConfigBone->GetPos());
		stateControl.SetTargetOrientation(rigConfigBone->GetRot());
		control = &stateControl;
		type = pragma::ik::RigConfigControl::Type::State;
	}
	else {
		auto &dragControl = m_ikSolver->AddDragControl(*rigConfigBone);
		dragControl.SetTargetPosition(rigConfigBone->GetPos());
		control = &dragControl;
		type = pragma::ik::RigConfigControl::Type::Drag;
	}
	auto &name = bone->name;
	using TComponent = IkSolverComponent;
	auto defGetSet = [this, &bone, rigConfigBone, &name, boneId](auto &ctrl) {
		using TControl = std::remove_reference_t<decltype(ctrl)>;

		auto posePropName = "control/" + name + "/pose";
		auto posPropName = "control/" + name + "/position";
		auto rotPropName = "control/" + name + "/rotation";

		auto coordMetaData = std::make_shared<ents::CoordinateTypeMetaData>();
		coordMetaData->space = umath::CoordinateSpace::Object;

		std::shared_ptr<ents::PoseComponentTypeMetaData> compMetaData = nullptr;
		if constexpr(std::is_same_v<TControl, pragma::ik::StateControl>) {
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

		if constexpr(std::is_same_v<TControl, pragma::ik::StateControl>) {
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
	};
	switch(type) {
	case pragma::ik::RigConfigControl::Type::State:
		defGetSet(static_cast<pragma::ik::StateControl &>(*control));
		break;
	case pragma::ik::RigConfigControl::Type::Drag:
		defGetSet(static_cast<pragma::ik::DragControl &>(*control));
		break;
	}
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
void IkSolverComponent::ResetIkRig() { InitializeSolver(); }
const std::shared_ptr<pragma::ik::Solver> &IkSolverComponent::GetIkSolver() const { return m_ikSolver; }
void IkSolverComponent::Solve()
{
	InvokeEventCallbacks(EVENT_UPDATE_IK);
	if(!m_updateRequired)
		return;
	m_updateRequired = false;

	// TODO: Reset pose?
	ResetIkBones();
	for(uint32_t i = 0; i < 5; ++i)
		m_ikSolver->Solve();
}
void IkSolverComponent::ResetIkBones()
{
	auto numBones = m_ikSolver->GetBoneCount();
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i) {
		auto *bone = m_ikSolver->GetBone(i);
		auto it = m_ikBoneIdToBoneId.find(i);
		if(it == m_ikBoneIdToBoneId.end() || !bone)
			continue;
		auto boneId = it->second;
		auto pose = GetReferenceBonePose(boneId);
		if(!pose.has_value())
			continue;
		bone->SetPos(pose->GetOrigin());
		bone->SetRot(pose->GetRotation());
	}
}
