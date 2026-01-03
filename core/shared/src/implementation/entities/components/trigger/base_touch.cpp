// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.triggers.base_touch;

using namespace pragma;

void BaseTouchComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseTouchComponent::EVENT_CAN_TRIGGER = registerEvent("CAN_TRIGGER", ComponentEventInfo::Type::Broadcast);
	baseTouchComponent::EVENT_ON_START_TOUCH = registerEvent("ON_START_TOUCH", ComponentEventInfo::Type::Broadcast);
	baseTouchComponent::EVENT_ON_END_TOUCH = registerEvent("ON_END_TOUCH", ComponentEventInfo::Type::Broadcast);
	baseTouchComponent::EVENT_ON_TRIGGER = registerEvent("ON_TRIGGER", ComponentEventInfo::Type::Broadcast);
	baseTouchComponent::EVENT_ON_TRIGGER_INITIALIZED = registerEvent("ON_TRIGGER_INITIALIZED", ComponentEventInfo::Type::Broadcast);
}
BaseTouchComponent::BaseTouchComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) { m_touching.reserve(10); }

void BaseTouchComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(basePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED, [this](std::reference_wrapper<ComponentEvent> evData) { OnPhysicsInitialized(); });
	BindEventUnhandled(basePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE, [this](std::reference_wrapper<ComponentEvent> evData) {
		UpdateTouch();

		if(!m_neverDisablePhysicsCallbacks) {
			// Note this *has* to be called before calling SetForcePhysicsAwakeCallbacksEnabled below!
			static_cast<CEPostPhysicsSimulate &>(evData.get()).keepAwake = false;

			auto pPhysComponent = GetEntity().GetPhysicsComponent();
			if(pPhysComponent)
				pPhysComponent->SetForcePhysicsAwakeCallbacksEnabled(false, false);
		}
	});
	BindEventUnhandled(baseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) { UpdatePhysics(); });

	auto &ent = GetEntity();

	auto whPhysComponent = ent.AddComponent("physics");
	if(whPhysComponent.valid())
		static_cast<BasePhysicsComponent &>(*whPhysComponent).SetCollisionCallbacksEnabled(true);
	ent.AddComponent("io");
	ent.AddComponent("model");
}
void BaseTouchComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	EndAllTouch();
}
void BaseTouchComponent::SetNeverDisablePhysicsCallbacks(bool b) { m_neverDisablePhysicsCallbacks = b; }
void BaseTouchComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto physComponent = ent.GetPhysicsComponent();
	physics::CollisionMask masks = physics::CollisionMask::None;
	if((m_triggerFlags & TriggerFlags::Everything) != TriggerFlags::None)
		masks = physics::CollisionMask::Dynamic | physics::CollisionMask::Generic;
	else {
		if((m_triggerFlags & TriggerFlags::Everything) != TriggerFlags::Players)
			masks |= physics::CollisionMask::Player;
		if((m_triggerFlags & TriggerFlags::Everything) != TriggerFlags::NPCs)
			masks |= physics::CollisionMask::NPC;
		if((m_triggerFlags & TriggerFlags::Everything) != TriggerFlags::Physics)
			masks |= physics::CollisionMask::Dynamic;
	}
	if(physComponent) {
		physComponent->SetCollisionFilterMask(masks);
		physComponent->SetCollisionFilterGroup(physics::CollisionMask::Trigger);
		physComponent->SetCollisionContactReportEnabled(true);
		auto *physObj = physComponent->GetPhysicsObject();
		if(physObj) {
			physObj->SetTrigger(true);
			auto &colObjs = physObj->GetCollisionObjects();
			auto it = std::find_if(colObjs.begin(), colObjs.end(), [](const util::TSharedHandle<physics::ICollisionObject> &hColObj) { return hColObj.IsValid() && !hColObj->IsTrigger(); });
			if(it != colObjs.end())
				Con::CWAR << "Trigger entity has non-trigger physics shapes!" << Con::endl;
		}
	}
	BroadcastEvent(baseTouchComponent::EVENT_ON_TRIGGER_INITIALIZED);
}
void BaseTouchComponent::UpdatePhysics()
{
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(!pPhysComponent)
		return;
	pPhysComponent->InitializePhysics(physics::PhysicsType::Static);
}
void BaseTouchComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto flags = ent.GetSpawnFlags();
	if(flags & SF_TRIGGER_EVERYTHING)
		m_triggerFlags |= TriggerFlags::Everything;
	if(flags & SF_TRIGGER_PLAYERS)
		m_triggerFlags |= TriggerFlags::Players;
	if(flags & SF_TRIGGER_NPCS)
		m_triggerFlags |= TriggerFlags::NPCs;
	if(flags & SF_TRIGGER_PHYSICS)
		m_triggerFlags |= TriggerFlags::Physics;
	UpdatePhysics();
}
util::EventReply BaseTouchComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == baseToggleComponent::EVENT_ON_TURN_OFF) {
		// Fire touch events for all actors that are currently touching the trigger
		auto num = m_touching.size();
		for(auto &touchInfo : m_touching) {
			auto bLastTouch = (--num == 0);
			FireEndTouchEvents(touchInfo, bLastTouch);
		}
	}
	else if(eventId == baseToggleComponent::EVENT_ON_TURN_ON) {
		// Fire touch events for all actors that are currently touching the trigger
		auto bFirstTouch = true;
		for(auto &touchInfo : m_touching) {
			FireStartTouchEvents(touchInfo, bFirstTouch);
			bFirstTouch = false;
		}
	}
	return util::EventReply::Unhandled;
}
void BaseTouchComponent::EndAllTouch()
{
	while(m_touching.empty() == false) {
		auto &touch = m_touching.back();
		if(touch.touch.entity.valid() == false) {
			m_touching.pop_back();
			continue;
		}
		EndTouch(*touch.touch.entity.get());
	}
}

void BaseTouchComponent::UpdateTouch()
{
	while(m_contactEventQueue.empty() == false) {
		auto &contactEvent = m_contactEventQueue.front();
		m_contactEventQueue.pop();

		if(contactEvent.contactTarget.valid() == false)
			continue;
		switch(contactEvent.eventType) {
		case ContactEvent::Event::StartTouch:
			{
				if(IsTouching(*contactEvent.contactTarget.get()))
					break;
				auto *ent = contactEvent.contactTarget.get();
				StartTouch(physics::PhysTouch {ent, contactEvent.contactTarget->CallOnRemove(FunctionCallback<void, ecs::BaseEntity *>::Create([this](ecs::BaseEntity *ent) { EndTouch(*ent); }))});
				break;
			}
		case ContactEvent::Event::EndTouch:
			EndTouch(*contactEvent.contactTarget.get());
			break;
		}
	}

	auto hEnt = GetEntity().GetHandle();
	for(unsigned int i = 0; i < m_contactReport.size(); i++) {
		OnContact(m_contactReport[i]);
		if(!hEnt.valid())
			return;
	}
	m_contactReport.clear();
}

void BaseTouchComponent::OnTouch(physics::PhysTouch &) {}
void BaseTouchComponent::OnContact(physics::ContactInfo &contact) {}
bool BaseTouchComponent::IsTouchEnabled() const { return true; }
void BaseTouchComponent::StartTouch(ecs::BaseEntity &entOther, physics::PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther)
{
	m_contactEventQueue.push({entOther.GetHandle(), ContactEvent::Event::StartTouch});

	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->SetForcePhysicsAwakeCallbacksEnabled(true, true, true);
}
void BaseTouchComponent::EndTouch(ecs::BaseEntity &entOther, physics::PhysObj &physOther, physics::ICollisionObject &objThis, physics::ICollisionObject &objOther)
{
	m_contactEventQueue.push({entOther.GetHandle(), ContactEvent::Event::EndTouch});

	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->SetForcePhysicsAwakeCallbacksEnabled(true, true, false);
}
void BaseTouchComponent::Contact(const physics::ContactInfo &contactInfo)
{
	m_contactReport.push_back(contactInfo);

	auto pPhysComponent = GetEntity().GetPhysicsComponent();
	if(pPhysComponent)
		pPhysComponent->SetForcePhysicsAwakeCallbacksEnabled(true);
}

bool BaseTouchComponent::CanTrigger(ecs::BaseEntity &ent)
{
	auto evCanTriggerData = CECanTriggerData {&ent};
	if(BroadcastEvent(baseTouchComponent::EVENT_CAN_TRIGGER, evCanTriggerData) == util::EventReply::Handled)
		return evCanTriggerData.canTrigger;
	if(evCanTriggerData.canTrigger == false)
		return false;
	return (m_triggerFlags & TriggerFlags::Everything) == TriggerFlags::Everything || ((m_triggerFlags & TriggerFlags::NPCs) != TriggerFlags::None && ent.IsNPC()) || ((m_triggerFlags & TriggerFlags::Players) != TriggerFlags::None && ent.IsPlayer())
	  || ((m_triggerFlags & TriggerFlags::Physics) != TriggerFlags::None && ent.IsNPC() == false && ent.IsPlayer() == false && ent.GetPhysicsComponent() && ent.GetPhysicsComponent()->GetPhysicsObject() != nullptr);
}
void BaseTouchComponent::OnStartTouch(ecs::BaseEntity &ent)
{
	BroadcastEvent(baseTouchComponent::EVENT_ON_START_TOUCH, CETouchData {ent});

	auto *l = ent.GetNetworkState()->GetGameState()->GetLuaState();
	auto &entThis = GetEntity();
	auto *pIoComponent = static_cast<BaseIOComponent *>(entThis.FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("onstarttouch", &ent);
}
void BaseTouchComponent::OnEndTouch(ecs::BaseEntity &ent)
{
	BroadcastEvent(baseTouchComponent::EVENT_ON_END_TOUCH, CETouchData {ent});

	auto *l = ent.GetNetworkState()->GetGameState()->GetLuaState();
	auto &entThis = GetEntity();
	auto *pIoComponent = static_cast<BaseIOComponent *>(entThis.FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("onendtouch", &ent);
}
void BaseTouchComponent::OnTrigger(ecs::BaseEntity &ent)
{
	BroadcastEvent(baseTouchComponent::EVENT_ON_TRIGGER, CETouchData {ent});

	auto *l = ent.GetNetworkState()->GetGameState()->GetLuaState();
	auto &entThis = GetEntity();
	auto *pIoComponent = static_cast<BaseIOComponent *>(entThis.FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("trigger", &ent);
}
void BaseTouchComponent::Trigger(ecs::BaseEntity &ent) { OnTrigger(ent); }

void BaseTouchComponent::FireStartTouchEvents(TouchInfo &touch, bool isFirstTouch)
{
	ecs::BaseEntity *ent = touch.touch.entity.get();
	touch.triggered = IsTouchEnabled() && CanTrigger(*ent);
	if(touch.triggered == false)
		return;
	auto &entThis = GetEntity();
	auto pPhysComponent = ent->GetPhysicsComponent();
	physics::PhysObj *phys = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
	auto hEnt = entThis.GetHandle();
	OnStartTouch(*ent);
	if(isFirstTouch) {
		auto *pIoComponent = static_cast<BaseIOComponent *>(entThis.FindComponent("io").get());
		if(pIoComponent != nullptr)
			pIoComponent->TriggerOutput("onstarttouchall", ent);
	}
	if(!hEnt.valid())
		return;
	if(pPhysComponent != nullptr && pPhysComponent->IsTrigger())
		Trigger(*ent);
}

void BaseTouchComponent::FireEndTouchEvents(TouchInfo &touch, bool isLastTouch)
{
	ecs::BaseEntity *ent = touch.touch.entity.get();
	if(!IsTouchEnabled() || touch.triggered == false)
		return;
	touch.triggered = false;
	auto hEnt = GetEntity().GetHandle();
	OnEndTouch(*ent);
	if(hEnt.valid())
		Trigger(*ent);
	if(isLastTouch) {
		auto *pIoComponent = static_cast<BaseIOComponent *>(GetEntity().FindComponent("io").get());
		if(pIoComponent != nullptr)
			pIoComponent->TriggerOutput("onendtouchall", ent);
	}
}

void BaseTouchComponent::StartTouch(const physics::PhysTouch &touch)
{
	ecs::BaseEntity *ent = const_cast<ecs::BaseEntity *>(touch.entity.get());
	if(ent == nullptr || IsTouching(*ent))
		return;
	auto bFirst = m_touching.empty();
	m_touching.push_back({touch, false});
	FireStartTouchEvents(m_touching.back(), bFirst);
}
void BaseTouchComponent::StartTouch(ecs::BaseEntity &ent)
{
	if(!CanTrigger(ent) || IsTouching(ent))
		return;
	auto touch = physics::PhysTouch(&ent, ent.CallOnRemove(FunctionCallback<void, ecs::BaseEntity *>::Create([this](ecs::BaseEntity *ent) { EndTouch(*ent); })));
	StartTouch(touch);
}
void BaseTouchComponent::SetTriggerFlags(TriggerFlags flags) { m_triggerFlags = flags; }
BaseTouchComponent::TriggerFlags BaseTouchComponent::GetTriggerFlags() const { return m_triggerFlags; }
bool BaseTouchComponent::IsTouching(ecs::BaseEntity &ent) const
{
	return std::find_if(m_touching.begin(), m_touching.end(), [&ent](const TouchInfo &touchInfo) { return touchInfo.touch.entity.get() == &ent; }) != m_touching.end();
}
void BaseTouchComponent::EndTouch(ecs::BaseEntity &ent)
{
	auto it = std::find_if(m_touching.begin(), m_touching.end(), [&ent](const TouchInfo &touchInfo) { return touchInfo.touch.entity.get() == &ent; });
	if(it == m_touching.end())
		return;
	FireEndTouchEvents(*it, m_touching.size() == 1);
	m_touching.erase(it);
}
const std::vector<BaseTouchComponent::TouchInfo> &BaseTouchComponent::GetTouchingInfo() const { return m_touching; }

////////////////

CECanTriggerData::CECanTriggerData(ecs::BaseEntity *ent) : entity(ent) {}
uint32_t CECanTriggerData::GetReturnCount() { return 1u; }
void CECanTriggerData::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -1))
		canTrigger = Lua::CheckBool(l, -1);
}
void CECanTriggerData::PushArguments(lua::State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}

////////////////

CETouchData::CETouchData(ecs::BaseEntity &ent) : entity(&ent) {}
void CETouchData::PushArguments(lua::State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject().push(l);
	else
		Lua::PushNil(l);
}
