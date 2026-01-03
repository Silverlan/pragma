// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.flex_merge;

using namespace pragma;

bool FlexMergeComponent::can_merge(const asset::Model &mdl, const asset::Model &mdlParent)
{
	auto &flexControllers = mdl.GetFlexControllers();
	for(auto &flexCon : flexControllers) {
		uint32_t id;
		if(mdlParent.GetFlexControllerId(flexCon.name, id))
			return true;
	}
	return false;
}

void FlexMergeComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { flexMergeComponent::EVENT_ON_TARGET_CHANGED = registerEvent("ON_TARGET_CHANGED", ComponentEventInfo::Type::Broadcast); }
void FlexMergeComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = FlexMergeComponent;

	{
		using TTarget = EntityURef;
		auto memberInfo = create_component_member_info<T, TTarget, static_cast<void (T::*)(const TTarget &)>(&T::SetTarget), static_cast<const TTarget &(T::*)() const>(&T::GetTarget)>("target", TTarget {});
		registerMember(std::move(memberInfo));
	}
}
FlexMergeComponent::FlexMergeComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void FlexMergeComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { SetTargetDirty(); });
	GetEntity().AddComponent("flex");
}
void FlexMergeComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void FlexMergeComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	SetTargetDirty();
}

void FlexMergeComponent::SetTarget(const EntityURef &target)
{
	m_target = target;
	SetTargetDirty();
	BroadcastEvent(flexMergeComponent::EVENT_ON_TARGET_CHANGED);
}
const EntityURef &FlexMergeComponent::GetTarget() const { return m_target; }

void FlexMergeComponent::SetTargetDirty(bool updateImmediately)
{
	m_flexC = pragma::ComponentHandle<BaseFlexComponent> {};
	m_flexCParent = pragma::ComponentHandle<BaseFlexComponent> {};
	m_flexControllerMap.clear();
	if(m_cbOnFlexControllerChanged.IsValid())
		m_cbOnFlexControllerChanged.Remove();
	if(m_cbOnFlexControllerComponentRemoved.IsValid())
		m_cbOnFlexControllerComponentRemoved.Remove();

	SetTickPolicy(TickPolicy::Always);
	if(updateImmediately)
		UpdateFlexControllerMappings();
}

void FlexMergeComponent::OnTick(double tDelta)
{
	BaseEntityComponent::OnTick(tDelta);
	UpdateFlexControllerMappings();
}

void FlexMergeComponent::UpdateFlexControllerMappings()
{
	auto &ent = GetEntity();
	auto *entTgt = m_target.GetEntity(GetGame());
	if(!entTgt)
		return;
	auto flexC = ent.FindComponent("flex");
	auto flexCTgt = entTgt->FindComponent("flex");
	if(flexC.expired() || flexCTgt.expired())
		return;
	auto &mdl = ent.GetModel();
	auto &mdlTgt = entTgt->GetModel();
	if(!mdl || !mdlTgt)
		return;
	auto &flexControllers = mdl->GetFlexControllers();
	for(size_t i = 0; i < flexControllers.size(); ++i) {
		auto &flexC = flexControllers[i];
		uint32_t idTgt;
		if(!mdlTgt->GetFlexControllerId(flexC.name, idTgt))
			continue;
		m_flexControllerMap[idTgt] = i;
	}
	m_flexC = flexC->GetHandle<BaseFlexComponent>();
	m_flexCParent = flexCTgt->GetHandle<BaseFlexComponent>();
	m_flexCParent->SetFlexControllerUpdateListenersEnabled(true);
	if(m_cbOnFlexControllerChanged.IsValid())
		m_cbOnFlexControllerChanged.Remove();
	if(m_cbOnFlexControllerComponentRemoved.IsValid())
		m_cbOnFlexControllerComponentRemoved.Remove();
	m_cbOnFlexControllerChanged = m_flexCParent->AddEventCallback(baseFlexComponent::EVENT_ON_FLEX_CONTROLLER_CHANGED, [this](std::reference_wrapper<ComponentEvent> ev) -> util::EventReply {
		auto &evFlex = static_cast<CEOnFlexControllerChanged &>(ev.get());
		ApplyFlexController(evFlex.flexControllerId, evFlex.value);
		return util::EventReply::Unhandled;
	});
	auto flexCId = m_flexCParent->GetComponentId();
	auto *genericC = entTgt->GetGenericComponent();
	if(genericC) {
		m_cbOnFlexControllerComponentRemoved = genericC->AddEventCallback(baseGenericComponent::EVENT_ON_ENTITY_COMPONENT_REMOVED, [this, flexCId](std::reference_wrapper<ComponentEvent> ev) -> util::EventReply {
			auto &evRemoved = static_cast<CEOnEntityComponentRemoved &>(ev.get());
			if(evRemoved.component.GetComponentId() == flexCId) {
				if(m_cbOnFlexControllerComponentRemoved.IsValid())
					m_cbOnFlexControllerComponentRemoved.Remove();
				SetTargetDirty(false);
			}
			return util::EventReply::Unhandled;
		});
	}
	MergeFlexControllers();

	SetTickPolicy(TickPolicy::Never);
}
void FlexMergeComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentAdded(component);
	auto *flexC = dynamic_cast<BaseFlexComponent *>(&component);
	if(flexC) {
		SetTickPolicy(TickPolicy::Always);
		UpdateFlexControllerMappings();
	}
}
void FlexMergeComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseEntityComponent::OnEntityComponentRemoved(component);
	auto *flexC = dynamic_cast<BaseFlexComponent *>(&component);
	if(flexC) {
		SetTickPolicy(TickPolicy::Always);
		UpdateFlexControllerMappings();
	}
}
void FlexMergeComponent::ApplyFlexController(animation::FlexControllerId flexCId, float value)
{
	if(m_flexC.expired())
		return;
	auto it = m_flexControllerMap.find(flexCId);
	if(it == m_flexControllerMap.end())
		return;
	m_flexC->SetFlexController(it->second, value);
}
void FlexMergeComponent::MergeFlexControllers()
{
	if(m_flexC.expired() || m_flexCParent.expired())
		return;
	for(auto &[idTgt, id] : m_flexControllerMap) {
		float val;
		if(!m_flexCParent->GetFlexController(idTgt, val))
			continue;
		m_flexC->SetFlexController(id, val);
	}
}
