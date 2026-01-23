// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_point_at_target;

using namespace pragma;

BasePointAtTargetComponent::BasePointAtTargetComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_pointAtTarget {EntityProperty::Create()} {}
void BasePointAtTargetComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "point_at_target", false))
			m_kvPointAtTargetName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	m_netEvSetPointAtTarget = SetupNetEvent("set_point_at_target");
}
void BasePointAtTargetComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	if(m_cbOnPoseChanged.IsValid())
		m_cbOnPoseChanged.Remove();
	if(m_cbOnPoseChangedThis.IsValid())
		m_cbOnPoseChangedThis.Remove();
}
void BasePointAtTargetComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_kvPointAtTargetName.empty() == false) {
		ecs::EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(), ecs::EntityIterator::FilterFlags::Default | ecs::EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvPointAtTargetName);
		auto it = entIt.begin();
		if(it != entIt.end())
			SetPointAtTarget(**it);
	}
	auto *transformC = dynamic_cast<BaseTransformComponent *>(GetEntity().AddComponent("transform").get());
	if(transformC) {
		m_cbOnPoseChangedThis = transformC->AddEventCallback(baseTransformComponent::EVENT_ON_POSE_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
			UpdatePose();
			return util::EventReply::Unhandled;
		});
	}
}
const PEntityProperty &BasePointAtTargetComponent::GetPointAtTargetProperty() const { return m_pointAtTarget; }
void BasePointAtTargetComponent::ClearPointAtTarget() { SetPointAtTarget(nullptr); }
void BasePointAtTargetComponent::SetPointAtTarget(ecs::BaseEntity &ent) { SetPointAtTarget(&ent); }
void BasePointAtTargetComponent::SetPointAtTarget(ecs::BaseEntity *ent)
{
	*m_pointAtTarget = (ent != nullptr) ? ent->GetHandle() : EntityHandle {};
	if(m_cbOnPoseChanged.IsValid())
		m_cbOnPoseChanged.Remove();
	if(ent == nullptr)
		return;

	auto transformC = static_cast<BaseTransformComponent *>(ent->AddComponent("transform").get());
	if(transformC == nullptr)
		return;
	m_cbOnPoseChanged = transformC->AddEventCallback(baseTransformComponent::EVENT_ON_POSE_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		UpdatePose();
		return util::EventReply::Unhandled;
	});
}
ecs::BaseEntity *BasePointAtTargetComponent::GetPointAtTarget() const { return m_pointAtTarget->GetValue().get(); }
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
