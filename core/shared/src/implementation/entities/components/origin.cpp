// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.origin;

using namespace pragma;

void OriginComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { originComponent::EVENT_ON_ORIGIN_CHANGED = registerEvent("ON_ORIGIN_CHANGED", ComponentEventInfo::Type::Explicit); }
void OriginComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = OriginComponent;

	using TPose = math::Transform;
	using TPos = Vector3;
	using TRot = Quat;

	constexpr auto *posePathName = "pose";
	auto poseComponentMetaData = pragma::util::make_shared<ents::PoseComponentTypeMetaData>();
	poseComponentMetaData->poseProperty = posePathName;

	auto poseMetaData = pragma::util::make_shared<ents::PoseTypeMetaData>();
	poseMetaData->posProperty = "pos";
	poseMetaData->rotProperty = "rot";

	auto memberInfoPose = create_component_member_info<T, TPose, static_cast<void (T::*)(const TPose &)>(&T::SetOriginPose), static_cast<const TPose &(T::*)() const>(&T::GetOriginPose)>(posePathName, TPose {});
	memberInfoPose.SetFlag(ComponentMemberFlags::HideInInterface);
	memberInfoPose.AddTypeMetaData(poseMetaData);
	registerMember(std::move(memberInfoPose));

	auto memberInfoPos = create_component_member_info<T, TPos, static_cast<void (T::*)(const TPos &)>(&T::SetOriginPos), static_cast<const TPos &(T::*)() const>(&T::GetOriginPos)>("pos", TPos {});
	memberInfoPos.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoPos));

	auto memberInfoRot = create_component_member_info<T, TRot, static_cast<void (T::*)(const TRot &)>(&T::SetOriginRot), static_cast<const TRot &(T::*)() const>(&T::GetOriginRot)>("rot", TRot {});
	memberInfoRot.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoRot));
}
OriginComponent::OriginComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void OriginComponent::Initialize() { BaseEntityComponent::Initialize(); }
void OriginComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void OriginComponent::SetOriginPose(const math::Transform &pose)
{
	m_origin = pose;
	BroadcastEvent(originComponent::EVENT_ON_ORIGIN_CHANGED);
}
void OriginComponent::SetOriginPos(const Vector3 &pos)
{
	m_origin.SetOrigin(pos);
	BroadcastEvent(originComponent::EVENT_ON_ORIGIN_CHANGED);
}
void OriginComponent::SetOriginRot(const Quat &rot)
{
	m_origin.SetRotation(rot);
	BroadcastEvent(originComponent::EVENT_ON_ORIGIN_CHANGED);
}

const math::Transform &OriginComponent::GetOriginPose() const { return m_origin; }
const Vector3 &OriginComponent::GetOriginPos() const { return m_origin.GetOrigin(); }
const Quat &OriginComponent::GetOriginRot() const { return m_origin.GetRotation(); }
