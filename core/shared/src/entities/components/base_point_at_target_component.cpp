/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_point_at_target_component.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/base_transform_component.hpp"

using namespace pragma;

BasePointAtTargetComponent::BasePointAtTargetComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_pointAtTarget {pragma::EntityProperty::Create()} {}
void BasePointAtTargetComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "point_at_target", false))
			m_kvPointAtTargetName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	m_netEvSetPointAtTarget = SetupNetEvent("set_point_at_target");
}
void BasePointAtTargetComponent::OnRemove()
{
	if(m_cbOnPoseChanged.IsValid())
		m_cbOnPoseChanged.Remove();
	if(m_cbOnPoseChangedThis.IsValid())
		m_cbOnPoseChangedThis.Remove();
}
void BasePointAtTargetComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_kvPointAtTargetName.empty() == false) {
		EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(), EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvPointAtTargetName);
		auto it = entIt.begin();
		if(it != entIt.end())
			SetPointAtTarget(**it);
	}
	auto *transformC = dynamic_cast<pragma::BaseTransformComponent *>(GetEntity().AddComponent("transform").get());
	if(transformC) {
		m_cbOnPoseChangedThis = transformC->AddEventCallback(BaseTransformComponent::EVENT_ON_POSE_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
			UpdatePose();
			return util::EventReply::Unhandled;
		});
	}
}
const pragma::PEntityProperty &BasePointAtTargetComponent::GetPointAtTargetProperty() const { return m_pointAtTarget; }
void BasePointAtTargetComponent::ClearPointAtTarget() { SetPointAtTarget(nullptr); }
void BasePointAtTargetComponent::SetPointAtTarget(BaseEntity &ent) { SetPointAtTarget(&ent); }
void BasePointAtTargetComponent::SetPointAtTarget(BaseEntity *ent)
{
	*m_pointAtTarget = (ent != nullptr) ? ent->GetHandle() : EntityHandle {};
	if(m_cbOnPoseChanged.IsValid())
		m_cbOnPoseChanged.Remove();
	if(ent == nullptr)
		return;

	auto transformC = static_cast<pragma::BaseTransformComponent *>(ent->AddComponent("transform").get());
	if(transformC == nullptr)
		return;
	m_cbOnPoseChanged = transformC->AddEventCallback(BaseTransformComponent::EVENT_ON_POSE_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		UpdatePose();
		return util::EventReply::Unhandled;
	});
}
BaseEntity *BasePointAtTargetComponent::GetPointAtTarget() const { return m_pointAtTarget->GetValue().get(); }
void BasePointAtTargetComponent::UpdatePose()
{
	auto *entPointAtTarget = GetPointAtTarget();
	if(entPointAtTarget == nullptr) {
		if(m_cbOnPoseChanged.IsValid())
			m_cbOnPoseChanged.Remove();
		return;
	}
	auto pTransformComponent = GetEntity().GetTransformComponent();
	if(!pTransformComponent)
		return;
	auto posTgt = entPointAtTarget->GetPosition();
	auto pos = pTransformComponent->GetPosition();
	pTransformComponent->SetAngles(uvec::to_angle(posTgt - pos));
}
