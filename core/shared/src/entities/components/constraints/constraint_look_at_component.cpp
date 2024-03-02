/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/constraints/constraint_look_at_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/logging.hpp"

using namespace pragma;

void ConstraintLookAtComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintLookAtComponent;
	{
		auto memberInfo = create_component_member_info<T, TrackAxis, &T::SetTrackAxis, &T::GetTrackAxis>("trackAxis", TrackAxis::Z);
		registerMember(std::move(memberInfo));
	}

	{
		using TUpTarget = pragma::EntityUComponentMemberRef;
		auto memberInfo = create_component_member_info<T, TUpTarget, static_cast<void (T::*)(const TUpTarget &)>(&T::SetUpTarget), static_cast<const TUpTarget &(T::*)() const>(&T::GetUpTarget)>("upTarget", TUpTarget {});
		registerMember(std::move(memberInfo));
	}
}

ConstraintLookAtComponent::ConstraintLookAtComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintLookAtComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(ConstraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ApplyConstraint(); });
	BindEventUnhandled(ConstraintComponent::EVENT_ON_DRIVER_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ResetDrivenRotation(); });
	BindEventUnhandled(ConstraintComponent::EVENT_ON_DRIVEN_OBJECT_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { ResetDrivenRotation(); });
}
void ConstraintLookAtComponent::SetUpTarget(const pragma::EntityUComponentMemberRef &drivenObject) { m_upTarget = drivenObject; }
const pragma::EntityUComponentMemberRef &ConstraintLookAtComponent::GetUpTarget() const { return m_upTarget; }
void ConstraintLookAtComponent::ResetDrivenRotation() { m_drivenObjectRotationInitialized = false; }
void ConstraintLookAtComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintLookAtComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
}

void ConstraintLookAtComponent::SetTrackAxis(TrackAxis axis) { m_trackAxis = axis; }
ConstraintLookAtComponent::TrackAxis ConstraintLookAtComponent::GetTrackAxis() const { return m_trackAxis; }

#include "pragma/model/model.h"
// TODO: Use Model::GetTwistAxisRotationOffset?
static std::array<Quat, umath::to_integral(ConstraintLookAtComponent::TrackAxis::Count)> g_axisRotations {
  Model::GetTwistAxisRotationOffset(pragma::SignedAxis::X),    // x+
  Model::GetTwistAxisRotationOffset(pragma::SignedAxis::Y),    // y+
  Model::GetTwistAxisRotationOffset(pragma::SignedAxis::Z),    // z+
  Model::GetTwistAxisRotationOffset(pragma::SignedAxis::NegX), // x-
  Model::GetTwistAxisRotationOffset(pragma::SignedAxis::NegY), // y-
  Model::GetTwistAxisRotationOffset(pragma::SignedAxis::NegZ)  // z-
};

static void rotate_towards_axis(Quat &rotation, const Vector3 &targetAxis, const Vector3 &upAxis, pragma::ConstraintLookAtComponent::TrackAxis eAxis)
{
	auto rotDriven = uquat::create_look_rotation(targetAxis, upAxis);
	rotDriven = rotDriven * g_axisRotations[umath::to_integral(eAxis)];
	rotation = rotDriven;
}

std::pair<pragma::BaseEntityComponent *, pragma::ComponentMemberIndex> ConstraintLookAtComponent::UpdateUpTarget()
{
	auto &game = GetGame();
	auto *upTargetC = m_upTarget.GetComponent(game);
	m_upTarget.UpdateMemberIndex(game);
	auto idxUpTarget = m_upTarget.GetMemberIndex();
	if(!upTargetC || idxUpTarget == pragma::INVALID_COMPONENT_MEMBER_INDEX)
		return {nullptr, INVALID_COMPONENT_MEMBER_INDEX};
	return {upTargetC, idxUpTarget};
}

std::optional<pragma::EntityUComponentMemberRef> ConstraintLookAtComponent::FindPoseProperty(const pragma::BaseEntityComponent &c, pragma::ComponentMemberIndex basePropIdx)
{
	auto *memberInfo = c.GetMemberInfo(basePropIdx);
	if(!memberInfo)
		return {};
	auto *metaDataPose = memberInfo ? memberInfo->FindTypeMetaData<pragma::ents::PoseTypeMetaData>() : nullptr;
	if(metaDataPose)
		return EntityUComponentMemberRef {c.GetEntity(), c.GetComponentId(), memberInfo->GetName()};
	auto *metaDataPoseComponent = memberInfo ? memberInfo->FindTypeMetaData<pragma::ents::PoseComponentTypeMetaData>() : nullptr;
	if(!metaDataPoseComponent)
		return {};
	auto idxPose = c.GetMemberIndex(metaDataPoseComponent->poseProperty);
	if(!idxPose)
		return {};
	auto *poseMemberInfo = c.GetMemberInfo(*idxPose);
	if(!poseMemberInfo)
		return {};
	return EntityUComponentMemberRef {c.GetEntity(), c.GetComponentId(), poseMemberInfo->GetName()};
}

void ConstraintLookAtComponent::ApplyConstraint()
{
	if(m_constraintC.expired())
		return;
	auto influence = m_constraintC->GetInfluence();
	auto &constraintInfo = m_constraintC->GetConstraintParticipants();
	if(!constraintInfo || influence == 0.f)
		return;
	if(!m_drivenObjectRotationInitialized) {
		m_drivenObjectRotationInitialized = true;

		auto *memberInfoDriver = constraintInfo->driverC->GetMemberInfo(constraintInfo->driverPropIdx);
		auto *metaDataPoseDriver = memberInfoDriver ? memberInfoDriver->FindTypeMetaData<pragma::ents::PoseTypeMetaData>() : nullptr;
		std::optional<pragma::ComponentMemberIndex> driverPosPropertyIndex {};
		if(metaDataPoseDriver)
			driverPosPropertyIndex = constraintInfo->driverC->GetMemberIndex(metaDataPoseDriver->posProperty);
		else
			driverPosPropertyIndex = constraintInfo->driverPropIdx;

		if(!driverPosPropertyIndex.has_value()) {
			spdlog::trace("Unable to initialize look_at constraint '{}' with driver property {} of driver '{}': Driver property is not valid!", GetEntity().ToString(), constraintInfo->driverPropIdx, constraintInfo->driverC->GetEntity().ToString());
			return;
		}
		memberInfoDriver = constraintInfo->driverC->GetMemberInfo(*driverPosPropertyIndex);
		assert(memberInfoDriver != nullptr);
		if(memberInfoDriver->type != pragma::ents::EntityMemberType::Vector3) {
			spdlog::trace("Unable to initialize look_at constraint '{}' with driver property {} of driver '{}': Driver property does not have supported type!", GetEntity().ToString(), constraintInfo->driverPropIdx, constraintInfo->driverC->GetEntity().ToString());
			return;
		}

		auto poseProp = FindPoseProperty(*constraintInfo->drivenObjectC, constraintInfo->drivenObjectPropIdx);
		if(!poseProp) {
			spdlog::trace("Unable to initialize look_at constraint '{}' with driven object property {} of driven object '{}': Property is not a valid pose property.", GetEntity().ToString(), constraintInfo->drivenObjectPropIdx, constraintInfo->drivenObjectC->GetEntity().ToString());
			return;
		}

		auto &game = GetGame();
		poseProp->UpdateMemberIndex(game);
		auto *drivenObject = poseProp->GetComponent(game);
		auto drivenObjectPoseIdx = poseProp->GetMemberIndex();
		assert(drivenObject != nullptr && drivenObject == constraintInfo->drivenObjectC.get());
		if(!drivenObject || drivenObject != constraintInfo->drivenObjectC.get()) {
			// Unreachable
			return;
		}

		auto *memberInfo = drivenObject->GetMemberInfo(drivenObjectPoseIdx);
		auto *metaData = memberInfo ? memberInfo->FindTypeMetaData<pragma::ents::PoseTypeMetaData>() : nullptr;
		if(!metaData) {
			spdlog::trace("Unable to initialize look_at constraint '{}' with driven object property {} of driven object '{}': Property has no pose type meta data.", GetEntity().ToString(), constraintInfo->drivenObjectPropIdx, constraintInfo->drivenObjectC->GetEntity().ToString());
			return;
		}
		m_drivenObjectPosition = {constraintInfo->drivenObjectC->GetEntity(), constraintInfo->drivenObjectC->GetComponentId(), metaData->posProperty};
		m_drivenObjectRotation = {constraintInfo->drivenObjectC->GetEntity(), constraintInfo->drivenObjectC->GetComponentId(), metaData->rotProperty};
		m_driverPosition = {constraintInfo->driverC->GetEntity(), constraintInfo->driverC->GetComponentId(), memberInfoDriver->GetName()};
	}
	auto &game = *GetEntity().GetNetworkState()->GetGameState();
	m_drivenObjectPosition.UpdateMemberIndex(game);
	m_drivenObjectRotation.UpdateMemberIndex(game);
	m_driverPosition.UpdateMemberIndex(game);
	auto *drivenObjectPosC = m_drivenObjectPosition.GetComponent(game);
	if(!drivenObjectPosC || drivenObjectPosC != constraintInfo->drivenObjectC.get())
		return;
	auto *drivenObjectRotC = m_drivenObjectRotation.GetComponent(game);
	if(!drivenObjectRotC || drivenObjectRotC != constraintInfo->drivenObjectC.get())
		return;
	auto idxDrivenObjectPos = m_drivenObjectPosition.GetMemberIndex();
	if(idxDrivenObjectPos == pragma::INVALID_COMPONENT_MEMBER_INDEX)
		return;
	auto idxDrivenObjectRot = m_drivenObjectRotation.GetMemberIndex();
	if(idxDrivenObjectRot == pragma::INVALID_COMPONENT_MEMBER_INDEX)
		return;
	auto idxDriverPos = m_driverPosition.GetMemberIndex();
	if(idxDriverPos == pragma::INVALID_COMPONENT_MEMBER_INDEX)
		return;

	Vector3 posDriven;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberPos(idxDrivenObjectPos, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriven);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}

	Vector3 posDriver;
	res = constraintInfo->driverC->GetTransformMemberPos(idxDriverPos, static_cast<umath::CoordinateSpace>(m_constraintC->GetDriverSpace()), posDriver);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driver of constraint '{}'.", constraintInfo->driverPropIdx, GetEntity().ToString());
		return;
	}

	auto dir = posDriver - posDriven;
	auto l = uvec::length(dir);
	if(l < 0.0001f)
		dir = uvec::FORWARD;
	else
		dir /= l;

	Quat curRot;
	res = constraintInfo->drivenObjectC->GetTransformMemberRot(idxDrivenObjectRot, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), curRot);

	auto [upTargetC, upTargetPropIdx] = UpdateUpTarget();
	auto upVec = uvec::UP;
	if(upTargetC) {
		Quat upTargetRot;
		if(upTargetC->GetTransformMemberRot(upTargetPropIdx, umath::CoordinateSpace::World, upTargetRot))
			upVec = uquat::up(upTargetRot);
	}

	auto rot = curRot;
	rotate_towards_axis(rot, dir, upVec, m_trackAxis);

	if(res)
		rot = uquat::slerp(curRot, rot, influence);

	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberRot(idxDrivenObjectRot, static_cast<umath::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rot);
}
