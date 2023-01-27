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
#include "pragma/model/model.h"
#include "pragma/logging.hpp"
#include <panima/skeleton.hpp>
#include <panima/bone.hpp>

using namespace pragma;

ComponentEventId IkSolverComponent::EVENT_INITIALIZE_SOLVER = pragma::INVALID_COMPONENT_ID;
ComponentEventId IkSolverComponent::EVENT_UPDATE_IK = pragma::INVALID_COMPONENT_ID;
void IkSolverComponent::RegisterEvents(pragma::EntityComponentManager &componentManager,TRegisterComponentEvent registerEvent)
{
	EVENT_INITIALIZE_SOLVER = registerEvent("INITIALIZE_SOLVER",ComponentEventInfo::Type::Broadcast);
	EVENT_UPDATE_IK = registerEvent("UPDATE_IK",ComponentEventInfo::Type::Explicit);
}
static void get_ik_rig(const ComponentMemberInfo &memberInfo,IkSolverComponent &component,pragma::ents::Element &value)
{
	value = component.GetIkRig();
}
void IkSolverComponent::RegisterMembers(pragma::EntityComponentManager &componentManager,TRegisterComponentMember registerMember)
{
	using T = IkSolverComponent;

	{
		using TRigConfig = pragma::ents::Element;
		auto memberInfo = create_component_member_info<
			T,TRigConfig,
			+[](const ComponentMemberInfo &memberInfo,T &component,const TRigConfig &value) {},
			&get_ik_rig
		>("RigConfig");
		registerMember(std::move(memberInfo));
	}
	{
		using TRigConfigFile = std::string;
		auto memberInfo = create_component_member_info<
			T,TRigConfigFile,
			static_cast<void(T::*)(const TRigConfigFile&)>(&T::SetIkRigFile),
			static_cast<const TRigConfigFile&(T::*)() const>(&T::GetIkRigFile)
		>("RigConfigFile","",AttributeSpecializationType::File);
		auto &metaData = memberInfo.AddMetaData();
		metaData["rootPath"] = "scripts/ik_rigs/";
		metaData["extensions"] = pragma::ik::RigConfig::get_supported_extensions();
		metaData["stripExtension"] = true;
		registerMember(std::move(memberInfo));
	}
}
IkSolverComponent::IkSolverComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{}
void IkSolverComponent::Initialize()
{
	BaseEntityComponent::Initialize();
}
void IkSolverComponent::SetIkRigFile(const std::string &RigConfigFile)
{
	m_ikRigFile = RigConfigFile;
	UpdateIkRigFile();
}
const std::string &IkSolverComponent::GetIkRigFile() const {return m_ikRigFile;}
void IkSolverComponent::UpdateIkRigFile()
{
	InitializeSolver(); // Clear Rig
	if(m_ikRigFile.empty())
		return;
	auto rigConfig = pragma::ik::RigConfig::load("scripts/ik_rigs/" +m_ikRigFile);
	if(rigConfig)
		AddIkSolverByRig(*rigConfig);
	UpdateIkRig();
}
void IkSolverComponent::InitializeSolver()
{
	m_ikSolver = std::make_unique<pragma::ik::Solver>(100,10);

	ClearMembers();
	OnMembersChanged();

	BroadcastEvent(EVENT_INITIALIZE_SOLVER);
}
bool IkSolverComponent::AddIkSolverByChain(const std::string &boneName,uint32_t chainLength)
{
	if(chainLength < 2)
		return false;
	auto &ent = GetEntity();
	auto &mdl = ent.GetModel();
	if(!mdl)
		return false;
	auto &skeleton = mdl->GetSkeleton();
	auto &ref = mdl->GetReference();

	std::vector<BoneId> ikChain;
	ikChain.reserve(chainLength);
	auto boneId = skeleton.LookupBone(boneName);
	if(boneId == -1)
		return false;

	// TODO: Remove existing solver?

	auto bone = skeleton.GetBone(boneId).lock();
	for(auto i=decltype(chainLength){0};i<chainLength;++i)
	{
		if(bone == nullptr)
			return false;
		ikChain.push_back(bone->ID);
		bone = bone->parent.lock();
	}

	auto rig = pragma::ik::RigConfig();
	for(auto id : ikChain)
	{
		auto bone = skeleton.GetBone(id).lock();
		assert(bone != nullptr);
		rig.AddBone(bone->name);
	}

	// Pin the top-most parent of the chain (e.g. shoulder)
	rig.SetBoneLocked(skeleton.GetBone(ikChain.front()).lock()->name,true);

	// Add handles for all other bones in the chain (e.g. forearm or hand)
	for(auto i=decltype(ikChain.size()){2u};i<ikChain.size();++i)
	{
		// We want to be able to control the rotation of the last element in the chain (the effector), but
		// not the other elements
		if(i == ikChain.size() -1)
			rig.AddControl(skeleton.GetBone(ikChain[i]).lock()->name,pragma::ik::RigConfigControl::Type::State);
		else
			rig.AddControl(skeleton.GetBone(ikChain[i]).lock()->name,pragma::ik::RigConfigControl::Type::Drag);
	}

	// Add generic ballsocket constraints with no twist
	for(auto i=decltype(ikChain.size()){1u};i<ikChain.size();++i)
	{
		// We need to allow some minor twisting to avoid instability
		rig.AddBallSocketConstraint(
			skeleton.GetBone(ikChain[i -1]).lock()->name,
			skeleton.GetBone(ikChain[i]).lock()->name,
			EulerAngles(-90,-90,-0.5),
			EulerAngles(90,90,0.5)
		);
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
	auto &ref = mdl->GetReference();
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId,pose))
		return;
	AddBone(boneId,pose,1.f,1.f);
}
void IkSolverComponent::AddDragControl(BoneId boneId)
{
	//return self:AddControl(boneId,true,false)
}
void IkSolverComponent::AddStateControl(BoneId boneId)
{

}
pragma::ik::Bone *IkSolverComponent::GetBone(BoneId boneId)
{
	auto it = m_boneIdToIkBoneId.find(boneId);
	if(it == m_boneIdToIkBoneId.end())
		return nullptr;
	return m_ikSolver->GetBone(it->second);
}
void IkSolverComponent::SetBoneLocked(BoneId boneId,bool locked)
{
	auto *bone = GetBone(boneId);
	if(!bone)
		return;
	bone->SetPinned(locked);
}
bool IkSolverComponent::AddIkSolverByRig(const pragma::ik::RigConfig &rigConfig)
{
	auto mdl = GetEntity().GetModel();
	if(!mdl)
		return false;
	auto &skeleton = mdl->GetSkeleton();
	for(auto &boneData : rigConfig.GetBones())
	{
		auto boneId = skeleton.LookupBone(boneData.name);
		if(boneId == -1)
			return false;
		AddSkeletalBone(boneId);
		if(boneData.locked)
			SetBoneLocked(boneId,true);
	}

	for(auto &controlData : rigConfig.GetControls())
	{
		auto boneId = skeleton.LookupBone(controlData.bone);
		if(boneId == -1)
			return false;
		switch(controlData.type)
		{
		case pragma::ik::RigConfigControl::Type::Drag:
			AddDragControl(boneId);
			break;
		case pragma::ik::RigConfigControl::Type::State:
			AddStateControl(boneId);
			break;
		}
	}

	for(auto &constraintData : rigConfig.GetConstraints())
	{
		auto boneId0 = skeleton.LookupBone(constraintData.bone0);
		auto boneId1 = skeleton.LookupBone(constraintData.bone1);
		if(boneId0 == -1 || boneId1 == -1)
			return false;
		switch(constraintData.type)
		{
		case pragma::ik::RigConfigConstraint::Type::Fixed:
			AddFixedConstraint(boneId0,boneId1);
			break;
		case pragma::ik::RigConfigConstraint::Type::Hinge:
			AddHingeConstraint(boneId0,boneId1,constraintData.minLimits.p,constraintData.maxLimits.p);
			break;
		case pragma::ik::RigConfigConstraint::Type::BallSocket:
			AddBallSocketConstraint(boneId0,boneId1,constraintData.minLimits,constraintData.maxLimits);
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
	auto &ref = mdl->GetReference();
	umath::ScaledTransform pose;
	if(!ref.GetBonePose(boneId,pose))
		return {};
	return pose;
}
void IkSolverComponent::AddFixedConstraint(BoneId boneId0,BoneId boneId1)
{
	auto itBone0 = m_boneIdToIkBoneId.find(boneId0);
	auto itBone1 = m_boneIdToIkBoneId.find(boneId1);
	if(itBone0 == m_boneIdToIkBoneId.end() || itBone1 == m_boneIdToIkBoneId.end())
		return;
	auto *bone0 = m_ikSolver->GetBone(itBone0->second);
	auto *bone1 = m_ikSolver->GetBone(itBone1->second);
	if(!bone0 || !bone1)
		return;
	auto refPose0 = GetReferenceBonePose(boneId0);
	auto refPose1 = GetReferenceBonePose(boneId1);
	if(!refPose0.has_value() || !refPose1.has_value())
		return;
	auto &rotBone0 = refPose0->GetRotation();
	auto &rotBone1 = refPose1->GetRotation();

	// Lock distance and rotation to the parent
	m_ikSolver->AddBallSocketJoint(*bone0,*bone1,bone1->GetPos());

	// Lock the angles
	auto &joint = m_ikSolver->AddAngularJoint(*bone0,*bone1);
	joint.SetRigidity(1'000.f);

	// Lock swing limit to 0
	//self.m_solver:AddSwingLimit(bone0,bone1,self:GetDirectionFromBoneParent(boneId1),self:GetDirectionFromBoneParent(boneId1),math.rad(5)):SetRigidity(1000)

	// Restrict twist
	//self.m_solver:AddTwistLimit(bone0,bone1,rotBone1:GetForward(),rotBone1:GetForward(),math.rad(5)):SetRigidity(1000)
}
void IkSolverComponent::AddHingeConstraint(BoneId boneId0,BoneId boneId1,umath::Degree minAngle,umath::Degree maxAngle)
{
	auto itBone0 = m_boneIdToIkBoneId.find(boneId0);
	auto itBone1 = m_boneIdToIkBoneId.find(boneId1);
	if(itBone0 == m_boneIdToIkBoneId.end() || itBone1 == m_boneIdToIkBoneId.end())
		return;
	auto *bone0 = m_ikSolver->GetBone(itBone0->second);
	auto *bone1 = m_ikSolver->GetBone(itBone1->second);
	if(!bone0 || !bone1)
		return;

	auto refPose0 = GetReferenceBonePose(boneId0);
	auto refPose1 = GetReferenceBonePose(boneId1);
	if(!refPose0.has_value() || !refPose1.has_value())
		return;
	auto &rotBone0 = refPose0->GetRotation();
	auto &rotBone1 = refPose1->GetRotation();

	// The IK system only allows us to specify a general swing limit (in any direction). Since we want to be able to specify it in each
	// direction independently, we have to shift the rotation axes accordingly.
	auto rotBone1WithOffset = rotBone1 *uquat::create(EulerAngles(-(maxAngle +minAngle),0,0));

	// BallSocket is required to ensure the distance and rotation to the parent is locked
	m_ikSolver->AddBallSocketJoint(*bone0,*bone1,bone1->GetPos());

	// Revolute joint to lock rotation to a single axis
	m_ikSolver->AddRevoluteJoint(*bone0,*bone1,uquat::right(rotBone0));

	// Apply the swing limit
	auto &swingLimit = m_ikSolver->AddSwingLimit(*bone0,*bone1,uquat::up(rotBone1WithOffset),uquat::up(rotBone1),umath::deg_to_rad(maxAngle -minAngle));
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
void IkSolverComponent::AddBallSocketConstraint(BoneId boneId0,BoneId boneId1,const EulerAngles &minLimits,const EulerAngles &maxLimits)
{
	auto itBone0 = m_boneIdToIkBoneId.find(boneId0);
	auto itBone1 = m_boneIdToIkBoneId.find(boneId1);
	if(itBone0 == m_boneIdToIkBoneId.end() || itBone1 == m_boneIdToIkBoneId.end())
		return;
	auto *bone0 = m_ikSolver->GetBone(itBone0->second);
	auto *bone1 = m_ikSolver->GetBone(itBone1->second);
	if(!bone0 || !bone1)
		return;

	auto refPose0 = GetReferenceBonePose(boneId0);
	auto refPose1 = GetReferenceBonePose(boneId1);
	if(!refPose0.has_value() || !refPose1.has_value())
		return;
	auto &rotBone0 = refPose0->GetRotation();
	auto &rotBone1 = refPose1->GetRotation();

	// The IK system only allows us to specify a general swing limit (in any direction). Since we want to be able to specify it in each
	// direction independently, we have to shift the rotation axes accordingly.
	auto rotBone1WithOffset = rotBone1 *uquat::create(EulerAngles(-(maxLimits.y +minLimits.y),-(maxLimits.p +minLimits.p),-(maxLimits.r +minLimits.r)));

	// BallSocket is required to ensure the distance and rotation to the parent is locked
	m_ikSolver->AddBallSocketJoint(*bone0,*bone1,bone1->GetPos());

	// Revolute joint to lock rotation to a single axis
	//self.m_solver:AddRevoluteJoint(bone0,bone1,rotBone0:GetUp()) -- Test

	// Apply the swing limit
	//self.m_solver:AddSwingLimit(bone0,bone1,rotBone1WithOffset:GetForward(),rotBone1:GetForward(),math.rad(maxLimits.y -minLimits.y)):SetRigidity(16)
	//self.m_solver:AddSwingLimit(bone0,bone1,rotBone1:GetForward(),rotBone1:GetForward(),math.rad(45)):SetRigidity(16)
	GetEntity().GetNetworkState()->GetGameState()->DrawLine(
		(GetReferenceBonePose(boneId1))->GetOrigin(),(GetReferenceBonePose(boneId1))->GetOrigin() +uquat::up(rotBone1) *-200.f,Color::Red,12.f
	);
	auto &ellipseSwingLimit = m_ikSolver->AddEllipseSwingLimit(
		*bone0,*bone1,
		uquat::forward(rotBone1WithOffset),uquat::forward(rotBone1),
		uquat::up(rotBone1WithOffset),
		umath::deg_to_rad(maxLimits.p -minLimits.p),umath::deg_to_rad(maxLimits.y -minLimits.y)
	);
	ellipseSwingLimit.SetRigidity(1'000);

	auto &twistLimit = m_ikSolver->AddTwistLimit(
		*bone0,*bone1,
		uquat::forward(rotBone1WithOffset),uquat::forward(rotBone1),
		umath::deg_to_rad(maxLimits.r -minLimits.r)
	);
	twistLimit.SetRigidity(1'000);

	//self:GetRigConfig():AddArray("constraints",udm.TYPE_ELEMENT)

	/*local pose0 = self:GetReferenceBonePose(boneId0)
	local pose1 = self:GetReferenceBonePose(boneId1)
	local rot = pose0:GetRotation()
	local angOffset = (maxLimits -minLimits) /2.0
	rot = rot *angOffset:ToQuaternion()

	local swingSpan = math.max(
		maxLimits.p -minLimits.p,
		maxLimits.y -minLimits.y
	)
	local twistSpan = maxLimits.r -minLimits.r

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
	if(!rig)
		return false;
	AddIkSolverByRig(*rig);
	OnMembersChanged();
	return true;
}
void IkSolverComponent::InitializeLuaObject(lua_State *l) {pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

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
pragma::ik::Bone *IkSolverComponent::GetIkBone(BoneId boneId)
{
	auto rigConfigBoneId = GetIkBoneId(boneId);
	if(!rigConfigBoneId.has_value())
		return nullptr;
	return m_ikSolver->GetBone(*rigConfigBoneId);
}
void IkSolverComponent::AddControl(BoneId boneId,const Vector3 &translation,const Quat *rotation)
{
	auto &mdl = GetEntity().GetModel();
	if(!mdl)
		return;
	auto &skeleton = mdl->GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(!bone)
		return;
	auto rigConfigBoneId = GetIkBoneId(boneId);
	if(rigConfigBoneId.has_value() == false)
		return;
	auto *rigConfigBone = m_ikSolver->GetBone(*rigConfigBoneId);
	if(!rigConfigBone || m_ikSolver->FindControl(*rigConfigBone) != nullptr)
		return;
	pragma::ik::IControl *control = nullptr;
	pragma::ik::RigConfigControl::Type type;
	if(rotation)
	{
		auto &stateControl = m_ikSolver->AddStateControl(*rigConfigBone);
		stateControl.SetTargetPosition(translation);
		stateControl.SetTargetOrientation(*rotation);
		control = &stateControl;
		type = pragma::ik::RigConfigControl::Type::State;
	}
	else
	{
		auto &dragControl = m_ikSolver->AddDragControl(*rigConfigBone);
		dragControl.SetTargetPosition(translation);
		control = &dragControl;
		type = pragma::ik::RigConfigControl::Type::Drag;
	}
	auto &name = bone->name;
	using TComponent = IkSolverComponent;
	auto defGetSet = [this,&bone,rigConfigBone,&name,boneId](auto &ctrl) {
		using TControl = std::remove_reference_t<decltype(ctrl)>;

		auto memberInfoPos = pragma::ComponentMemberInfo::CreateDummy();
		memberInfoPos.SetName("control/" +name +"/position");
		memberInfoPos.type = ents::EntityMemberType::Vector3;
		memberInfoPos.userIndex = boneId;
		using TValue = Vector3;
		memberInfoPos.SetGetterFunction<TComponent,TValue,static_cast<void(*)(const pragma::ComponentMemberInfo&,TComponent&,TValue&)>(
			[](const pragma::ComponentMemberInfo &memberInfo,TComponent &component,TValue &outValue) {
			auto it = component.m_ikControls.find(memberInfo.userIndex);
			if(it == component.m_ikControls.end())
			{
				outValue = {};
				return;
			}
			outValue = static_cast<TControl*>(it->second.get())->GetTargetPosition();
		})>();
		memberInfoPos.SetSetterFunction<TComponent,TValue,static_cast<void(*)(const pragma::ComponentMemberInfo&,TComponent&,const TValue&)>(
			[](const pragma::ComponentMemberInfo &memberInfo,TComponent &component,const TValue &value) {
			auto it = component.m_ikControls.find(memberInfo.userIndex);
			if(it == component.m_ikControls.end())
				return;
			static_cast<TControl*>(it->second.get())->SetTargetPosition(value);
			component.m_updateRequired = true;
		})>();
		RegisterMember(std::move(memberInfoPos));
		ctrl.SetTargetPosition(rigConfigBone->GetPos());

		if constexpr(std::is_same_v<TControl,pragma::ik::StateControl>)
		{
			auto memberInfoRot = pragma::ComponentMemberInfo::CreateDummy();
			memberInfoRot.SetName("control/" +name +"/rotation");
			memberInfoRot.type = ents::EntityMemberType::Quaternion;
			memberInfoRot.userIndex = boneId;
			using TValue = Quat;
			memberInfoRot.SetGetterFunction<TComponent,TValue,static_cast<void(*)(const pragma::ComponentMemberInfo&,TComponent&,TValue&)>(
				[](const pragma::ComponentMemberInfo &memberInfo,TComponent &component,TValue &outValue) {
				auto it = component.m_ikControls.find(memberInfo.userIndex);
				if(it == component.m_ikControls.end())
				{
					outValue = {};
					return;
				}
				outValue = static_cast<TControl*>(it->second.get())->GetTargetOrientation();
			})>();
			memberInfoRot.SetSetterFunction<TComponent,TValue,static_cast<void(*)(const pragma::ComponentMemberInfo&,TComponent&,const TValue&)>(
				[](const pragma::ComponentMemberInfo &memberInfo,TComponent &component,const TValue &value) {
				auto it = component.m_ikControls.find(memberInfo.userIndex);
				if(it == component.m_ikControls.end())
					return;
				static_cast<TControl*>(it->second.get())->SetTargetOrientation(value);
				component.m_updateRequired = true;
			})>();
			RegisterMember(std::move(memberInfoRot));
			ctrl.SetTargetOrientation(rigConfigBone->GetRot());
		}
	};
	auto memberInfoLocked = pragma::ComponentMemberInfo::CreateDummy();
	memberInfoLocked.SetName("control/" +name +"/locked");
	memberInfoLocked.type = ents::EntityMemberType::Boolean;
	memberInfoLocked.userIndex = boneId;
	using TValue = bool;
	memberInfoLocked.SetGetterFunction<TComponent,TValue,static_cast<void(*)(const pragma::ComponentMemberInfo&,TComponent&,TValue&)>(
		[](const pragma::ComponentMemberInfo &memberInfo,TComponent &component,TValue &outValue) {
		auto it = component.m_boneIdToIkBoneId.find(memberInfo.userIndex);
		if(it == component.m_boneIdToIkBoneId.end())
		{
			outValue = false;
			return;
		}
		auto *bone = component.m_ikSolver->GetBone(it->second);
		if(!bone)
		{
			outValue = false;
			return;
		}
		outValue = bone->IsPinned();
	})>();
	memberInfoLocked.SetSetterFunction<TComponent,TValue,static_cast<void(*)(const pragma::ComponentMemberInfo&,TComponent&,const TValue&)>(
		[](const pragma::ComponentMemberInfo &memberInfo,TComponent &component,const TValue &value) {
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
	OnMembersChanged();
}
pragma::ik::Bone *IkSolverComponent::AddBone(BoneId boneId,const umath::Transform &pose,float radius,float length)
{
	auto rigConfigBone = GetIkBone(boneId);
	if(rigConfigBone)
		return rigConfigBone;
	IkBoneId rigConfigBoneId;
	rigConfigBone = &m_ikSolver->AddBone(pose.GetOrigin(),pose.GetRotation(),radius,length,&rigConfigBoneId);
	m_boneIdToIkBoneId[boneId] = rigConfigBoneId;
	m_ikBoneIdToBoneId[rigConfigBoneId] = boneId;
	return rigConfigBone;
}
void IkSolverComponent::Solve()
{
	InvokeEventCallbacks(EVENT_UPDATE_IK);
	if(!m_updateRequired)
		return;
	m_updateRequired = false;

	// TODO: Reset pose?
	ResetIkBones();
	for(uint32_t i=0;i<5;++i)
		m_ikSolver->Solve();
}
void IkSolverComponent::ResetIkBones()
{
	auto numBones = m_ikSolver->GetBoneCount();
	for(auto i=decltype(numBones){0u};i<numBones;++i)
	{
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
