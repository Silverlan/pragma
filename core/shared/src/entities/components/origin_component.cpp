/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/origin_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/component_member_flags.hpp"

using namespace pragma;

ComponentEventId OriginComponent::EVENT_ON_ORIGIN_CHANGED = pragma::INVALID_COMPONENT_ID;
void OriginComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_ORIGIN_CHANGED = registerEvent("ON_ORIGIN_CHANGED", ComponentEventInfo::Type::Explicit); }
void OriginComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = OriginComponent;

	using TPose = umath::Transform;
	using TPos = Vector3;
	using TRot = Quat;

	constexpr auto *posePathName = "pose";
	auto poseComponentMetaData = std::make_shared<ents::PoseComponentTypeMetaData>();
	poseComponentMetaData->poseProperty = posePathName;

	auto poseMetaData = std::make_shared<ents::PoseTypeMetaData>();
	poseMetaData->posProperty = "pos";
	poseMetaData->rotProperty = "rot";

	auto memberInfoPose = create_component_member_info<T, TPose, static_cast<void (T::*)(const TPose &)>(&T::SetOriginPose), static_cast<const TPose &(T::*)() const>(&T::GetOriginPose)>(posePathName, TPose {});
	memberInfoPose.SetFlag(pragma::ComponentMemberFlags::HideInInterface);
	memberInfoPose.AddTypeMetaData(poseMetaData);
	registerMember(std::move(memberInfoPose));

	auto memberInfoPos = create_component_member_info<T, TPos, static_cast<void (T::*)(const TPos &)>(&T::SetOriginPos), static_cast<const TPos &(T::*)() const>(&T::GetOriginPos)>("pos", TPos {});
	memberInfoPos.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoPos));

	auto memberInfoRot = create_component_member_info<T, TRot, static_cast<void (T::*)(const TRot &)>(&T::SetOriginRot), static_cast<const TRot &(T::*)() const>(&T::GetOriginRot)>("rot", TRot {});
	memberInfoRot.AddTypeMetaData(poseComponentMetaData);
	registerMember(std::move(memberInfoRot));
}
OriginComponent::OriginComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void OriginComponent::Initialize() { BaseEntityComponent::Initialize(); }
void OriginComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void OriginComponent::SetOriginPose(const umath::Transform &pose)
{
	m_origin = pose;
	BroadcastEvent(EVENT_ON_ORIGIN_CHANGED);
}
void OriginComponent::SetOriginPos(const Vector3 &pos)
{
	m_origin.SetOrigin(pos);
	BroadcastEvent(EVENT_ON_ORIGIN_CHANGED);
}
void OriginComponent::SetOriginRot(const Quat &rot)
{
	m_origin.SetRotation(rot);
	BroadcastEvent(EVENT_ON_ORIGIN_CHANGED);
}

const umath::Transform &OriginComponent::GetOriginPose() const { return m_origin; }
const Vector3 &OriginComponent::GetOriginPos() const { return m_origin.GetOrigin(); }
const Quat &OriginComponent::GetOriginRot() const { return m_origin.GetRotation(); }
