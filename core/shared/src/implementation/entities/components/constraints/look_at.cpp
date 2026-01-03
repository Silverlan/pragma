// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.components.constraints.look_at;

using namespace pragma;

void ConstraintLookAtComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = ConstraintLookAtComponent;
	{
		auto memberInfo = create_component_member_info<T, TrackAxis, &T::SetTrackAxis, &T::GetTrackAxis>("trackAxis", TrackAxis::Z);
		registerMember(std::move(memberInfo));
	}

	{
		using TUpTarget = EntityUComponentMemberRef;
		auto memberInfo = create_component_member_info<T, TUpTarget, static_cast<void (T::*)(const TUpTarget &)>(&T::SetUpTarget), static_cast<const TUpTarget &(T::*)() const>(&T::GetUpTarget)>("upTarget", TUpTarget {});
		registerMember(std::move(memberInfo));
	}
}

ConstraintLookAtComponent::ConstraintLookAtComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void ConstraintLookAtComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	GetEntity().AddComponent<ConstraintComponent>();
	BindEventUnhandled(constraintComponent::EVENT_APPLY_CONSTRAINT, [this](std::reference_wrapper<ComponentEvent> evData) { ApplyConstraint(); });
	BindEventUnhandled(constraintComponent::EVENT_ON_DRIVER_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { ResetDrivenRotation(); });
	BindEventUnhandled(constraintComponent::EVENT_ON_DRIVEN_OBJECT_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { ResetDrivenRotation(); });
}
void ConstraintLookAtComponent::SetUpTarget(const EntityUComponentMemberRef &drivenObject) { m_upTarget = drivenObject; }
const EntityUComponentMemberRef &ConstraintLookAtComponent::GetUpTarget() const { return m_upTarget; }
void ConstraintLookAtComponent::ResetDrivenRotation() { m_drivenObjectRotationInitialized = false; }
void ConstraintLookAtComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ConstraintLookAtComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(ConstraintComponent))
		m_constraintC = component.GetHandle<ConstraintComponent>();
}

void ConstraintLookAtComponent::SetTrackAxis(TrackAxis axis) { m_trackAxis = axis; }
ConstraintLookAtComponent::TrackAxis ConstraintLookAtComponent::GetTrackAxis() const { return m_trackAxis; }

// TODO: Use Model::GetTwistAxisRotationOffset?
static std::array<Quat, math::to_integral(ConstraintLookAtComponent::TrackAxis::Count)> g_axisRotations {
  asset::Model::GetTwistAxisRotationOffset(SignedAxis::X),    // x+
  asset::Model::GetTwistAxisRotationOffset(SignedAxis::Y),    // y+
  asset::Model::GetTwistAxisRotationOffset(SignedAxis::Z),    // z+
  asset::Model::GetTwistAxisRotationOffset(SignedAxis::NegX), // x-
  asset::Model::GetTwistAxisRotationOffset(SignedAxis::NegY), // y-
  asset::Model::GetTwistAxisRotationOffset(SignedAxis::NegZ)  // z-
};

static void rotate_towards_axis(Quat &rotation, const Vector3 &targetAxis, const Vector3 &upAxis, ConstraintLookAtComponent::TrackAxis eAxis)
{
	auto rotDriven = uquat::create_look_rotation(targetAxis, upAxis);
	rotDriven = rotDriven * g_axisRotations[math::to_integral(eAxis)];
	rotation = rotDriven;
}

std::pair<BaseEntityComponent *, ComponentMemberIndex> ConstraintLookAtComponent::UpdateUpTarget()
{
	auto &game = GetGame();
	auto *upTargetC = m_upTarget.GetComponent(game);
	m_upTarget.UpdateMemberIndex(game);
	auto idxUpTarget = m_upTarget.GetMemberIndex();
	if(!upTargetC || idxUpTarget == INVALID_COMPONENT_MEMBER_INDEX)
		return {nullptr, INVALID_COMPONENT_MEMBER_INDEX};
	return {upTargetC, idxUpTarget};
}

std::optional<EntityUComponentMemberRef> ConstraintLookAtComponent::FindPoseProperty(const BaseEntityComponent &c, ComponentMemberIndex basePropIdx)
{
	auto *memberInfo = c.GetMemberInfo(basePropIdx);
	if(!memberInfo)
		return {};
	auto *metaDataPose = memberInfo ? memberInfo->FindTypeMetaData<ents::PoseTypeMetaData>() : nullptr;
	if(metaDataPose)
		return EntityUComponentMemberRef {c.GetEntity(), c.GetComponentId(), memberInfo->GetName()};
	auto *metaDataPoseComponent = memberInfo ? memberInfo->FindTypeMetaData<ents::PoseComponentTypeMetaData>() : nullptr;
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
		auto *metaDataPoseDriver = memberInfoDriver ? memberInfoDriver->FindTypeMetaData<ents::PoseTypeMetaData>() : nullptr;
		std::optional<ComponentMemberIndex> driverPosPropertyIndex {};
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
		if(memberInfoDriver->type != ents::EntityMemberType::Vector3) {
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
		auto *metaData = memberInfo ? memberInfo->FindTypeMetaData<ents::PoseTypeMetaData>() : nullptr;
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
	if(idxDrivenObjectPos == INVALID_COMPONENT_MEMBER_INDEX)
		return;
	auto idxDrivenObjectRot = m_drivenObjectRotation.GetMemberIndex();
	if(idxDrivenObjectRot == INVALID_COMPONENT_MEMBER_INDEX)
		return;
	auto idxDriverPos = m_driverPosition.GetMemberIndex();
	if(idxDriverPos == INVALID_COMPONENT_MEMBER_INDEX)
		return;

	Vector3 posDriven;
	auto res = constraintInfo->drivenObjectC->GetTransformMemberPos(idxDrivenObjectPos, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), posDriven);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driven object of constraint '{}'.", constraintInfo->drivenObjectPropIdx, GetEntity().ToString());
		return;
	}

	Vector3 posDriver;
	res = constraintInfo->driverC->GetTransformMemberPos(idxDriverPos, static_cast<math::CoordinateSpace>(m_constraintC->GetDriverSpace()), posDriver);
	if(!res) {
		spdlog::trace("Failed to transform component property value for property {} for driver of constraint '{}'.", constraintInfo->driverPropIdx, GetEntity().ToString());
		return;
	}

	auto dir = posDriver - posDriven;
	auto l = uvec::length(dir);
	if(l < 0.0001f)
		dir = uvec::PRM_FORWARD;
	else
		dir /= l;

	Quat curRot;
	res = constraintInfo->drivenObjectC->GetTransformMemberRot(idxDrivenObjectRot, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), curRot);

	auto [upTargetC, upTargetPropIdx] = UpdateUpTarget();
	auto upVec = uvec::PRM_UP;
	if(upTargetC) {
		Quat upTargetRot;
		if(upTargetC->GetTransformMemberRot(upTargetPropIdx, math::CoordinateSpace::World, upTargetRot))
			upVec = uquat::up(upTargetRot);
	}

	auto rot = curRot;
	rotate_towards_axis(rot, dir, upVec, m_trackAxis);

	if(res)
		rot = uquat::slerp(curRot, rot, influence);

	const_cast<BaseEntityComponent &>(*constraintInfo->drivenObjectC).SetTransformMemberRot(idxDrivenObjectRot, static_cast<math::CoordinateSpace>(m_constraintC->GetDrivenObjectSpace()), rot);
}
