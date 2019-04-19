#include "stdafx_shared.h"
#include "pragma/entities/components/base_point_at_target_component.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/components/base_transform_component.hpp"

using namespace pragma;

BasePointAtTargetComponent::BasePointAtTargetComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_pointAtTarget{pragma::EntityProperty::Create()}
{}
void BasePointAtTargetComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"point_at_target",false))
			m_kvPointAtTargetName = kvData.value;
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	GetEntity().AddComponent<LogicComponent>();
	m_netEvSetPointAtTarget = SetupNetEvent("set_point_at_target");
}
void BasePointAtTargetComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	if(m_kvPointAtTargetName.empty() == false)
	{
		EntityIterator entIt {*GetEntity().GetNetworkState()->GetGameState(),EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
		entIt.AttachFilter<EntityIteratorFilterEntity>(m_kvPointAtTargetName);
		auto it = entIt.begin();
		if(it != entIt.end())
			SetPointAtTarget(**it);
	}
}
const pragma::PEntityProperty &BasePointAtTargetComponent::GetPointAtTargetProperty() const {return m_pointAtTarget;}
void BasePointAtTargetComponent::ClearPointAtTarget() {SetPointAtTarget(nullptr);}
void BasePointAtTargetComponent::SetPointAtTarget(BaseEntity &ent) {SetPointAtTarget(&ent);}
void BasePointAtTargetComponent::SetPointAtTarget(BaseEntity *ent)
{
	*m_pointAtTarget = (ent != nullptr) ? ent->GetHandle() : EntityHandle{};
	if(m_cbTick.IsValid())
		m_cbTick.Remove();
	if(ent == nullptr)
		return;
	m_cbTick = BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto *entPointAtTarget = GetPointAtTarget();
		if(entPointAtTarget == nullptr)
		{
			if(m_cbTick.IsValid())
				m_cbTick.Remove();
			return;
		}
		auto pTransformComponent = GetEntity().GetTransformComponent();
		if(pTransformComponent.expired())
			return;
		auto posTgt = entPointAtTarget->GetPosition();
		auto pos = pTransformComponent->GetPosition();
		pTransformComponent->SetAngles(uvec::to_angle(posTgt -pos));
	});
}
BaseEntity *BasePointAtTargetComponent::GetPointAtTarget() const {return m_pointAtTarget->GetValue().get();}
