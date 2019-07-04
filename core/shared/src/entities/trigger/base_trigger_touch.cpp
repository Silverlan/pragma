#include "stdafx_shared.h"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include "pragma/entities/baseentity.h"
#include "pragma/physics/physobj.h"
#include "pragma/physics/collisionmasks.h"
#include "pragma/physics/shape.hpp"
#include "pragma/entities/trigger/trigger_spawnflags.h"
#include "pragma/lua/luafunction_call.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/basetoggle.h"

using namespace pragma;

ComponentEventId BaseTouchComponent::EVENT_CAN_TRIGGER = INVALID_COMPONENT_ID;
ComponentEventId BaseTouchComponent::EVENT_ON_START_TOUCH = INVALID_COMPONENT_ID;
ComponentEventId BaseTouchComponent::EVENT_ON_END_TOUCH = INVALID_COMPONENT_ID;
ComponentEventId BaseTouchComponent::EVENT_ON_TRIGGER = INVALID_COMPONENT_ID;
void BaseTouchComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_CAN_TRIGGER = componentManager.RegisterEvent("CAN_TRIGGER");
	EVENT_ON_START_TOUCH = componentManager.RegisterEvent("ON_START_TOUCH");
	EVENT_ON_END_TOUCH = componentManager.RegisterEvent("ON_END_TOUCH");
	EVENT_ON_TRIGGER = componentManager.RegisterEvent("ON_TRIGGER");
}
BaseTouchComponent::BaseTouchComponent(BaseEntity &ent)
	: BaseEntityComponent(ent)
{
	m_touchReport.reserve(10);
	m_touching.reserve(10);
	m_endTouch.reserve(10);
}

void BaseTouchComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_PHYSICS_INITIALIZED,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnPhysicsInitialized();
	});
	BindEventUnhandled(BasePhysicsComponent::EVENT_ON_POST_PHYSICS_SIMULATE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		UpdateTouch();
	});

	auto &ent = GetEntity();

	ent.AddComponent("toggle");
	auto whPhysComponent = ent.AddComponent("physics");
	if(whPhysComponent.valid())
		static_cast<BasePhysicsComponent&>(*whPhysComponent).SetCollisionCallbacksEnabled(true);
	ent.AddComponent("io");
	ent.AddComponent("model");
}
void BaseTouchComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	EndAllTouch();
}
void BaseTouchComponent::OnPhysicsInitialized()
{
	auto &ent = GetEntity();
	auto physComponent = ent.GetPhysicsComponent();
	CollisionMask masks = CollisionMask::None;
	if((m_triggerFlags &TriggerFlags::Everything) != TriggerFlags::None)
		masks = CollisionMask::Dynamic | CollisionMask::Generic;
	else
	{
		if((m_triggerFlags &TriggerFlags::Everything) != TriggerFlags::Players)
			masks |= CollisionMask::Player;
		if((m_triggerFlags &TriggerFlags::Everything) != TriggerFlags::NPCs)
			masks |= CollisionMask::NPC;
		if((m_triggerFlags &TriggerFlags::Everything) != TriggerFlags::Physics)
			masks |= CollisionMask::Dynamic;
	}
	if(physComponent.valid())
	{
		physComponent->SetCollisionFilterMask(masks);
		physComponent->SetCollisionFilterGroup(CollisionMask::Trigger);
		auto *physObj = physComponent->GetPhysicsObject();
		if(physObj)
		{
			auto &colObjs = physObj->GetCollisionObjects();
			auto it = std::find_if(colObjs.begin(),colObjs.end(),[](const util::TSharedHandle<pragma::physics::ICollisionObject> &hColObj) {
				return hColObj.IsValid() && hColObj->GetCollisionShape() && hColObj->GetCollisionShape()->IsTrigger();
			});
			if(it != colObjs.end())
				Con::cwar<<"WARNING: Trigger entity has non-trigger physics shapes!"<<Con::endl;
		}
	}
}
void BaseTouchComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto flags = ent.GetSpawnFlags();
	if(flags &SF_TRIGGER_EVERYTHING)
		m_triggerFlags |= TriggerFlags::Everything;
	if(flags &SF_TRIGGER_PLAYERS)
		m_triggerFlags |= TriggerFlags::Players;
	if(flags &SF_TRIGGER_NPCS)
		m_triggerFlags |= TriggerFlags::NPCs;
	if(flags &SF_TRIGGER_PHYSICS)
		m_triggerFlags |= TriggerFlags::Physics;

	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent.valid())
		pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
}
util::EventReply BaseTouchComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
		EndAllTouch();
	return util::EventReply::Unhandled;
}
void BaseTouchComponent::EndAllTouch()
{
	for(auto it=m_touching.begin();it!=m_touching.end();)
		it = RemoveTouch(it);
	m_touching.clear();
}

void BaseTouchComponent::UpdateTouch()
{
	auto hEnt = GetEntity().GetHandle();
	for(int i=0;i<m_touching.size();i++)
	{
		PhysTouch &touch = m_touching[i];
		if(!touch.entity.IsValid()) // This shouldn't happen
		{
			m_touching.erase(m_touching.begin() +i);
			i--;
			continue;
		}
		bool bIsTouching = false;
		for(unsigned int j=0;j<m_touchReport.size();j++)
		{
			PhysTouch &touchOther = m_touchReport[j];
			if(touchOther.entity.get() == touch.entity.get())
			{
				bIsTouching = true;
				m_touchReport.erase(m_touchReport.begin() +j);
				break;
			}
		}
		if(bIsTouching == false)
		{
			m_endTouch.push_back(touch);
			m_touching.erase(m_touching.begin() +i);
			i--;
		}
		else
		{
			OnTouch(touch);
			if(!hEnt.IsValid())
				return;
		}
	}
	for(auto it=m_endTouch.begin();it!=m_endTouch.end();++it)
	{
		auto &touch = *it;
		auto pPhysComponent = touch.entity->GetPhysicsComponent();
		EndTouch(touch.entity.get(),pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr);
		if(!hEnt.IsValid())
			return;
	}
	m_endTouch.clear();
	for(unsigned int i=0;i<m_touchReport.size();i++)
	{
		PhysTouch &touch = m_touchReport[i];
		StartTouch(touch);
		if(!hEnt.IsValid())
			return;
	}
	m_touchReport.clear();
	for(unsigned int i=0;i<m_contactReport.size();i++)
	{
		OnContact(m_contactReport[i]);
		if(!hEnt.IsValid())
			return;
	}
	m_contactReport.clear();
}

void BaseTouchComponent::OnTouch(PhysTouch&) {}
void BaseTouchComponent::OnContact(PhysContact&) {}
bool BaseTouchComponent::IsTouchEnabled() const {return true;}
void BaseTouchComponent::Touch(BaseEntity *entOther,PhysObj *physOther,pragma::physics::ICollisionObject*,pragma::physics::ICollisionObject*)
{
	if(!IsTouchEnabled() || !CanTrigger(entOther,physOther))
		return;
	for(unsigned int i=0;i<m_touchReport.size();i++)
	{
		if(m_touchReport[i].entity.get() == entOther)
			return;
	}
	m_touchReport.push_back(PhysTouch(
		entOther,
		entOther->CallOnRemove(
			FunctionCallback<void,BaseEntity*>::Create(
				std::bind(static_cast<void(BaseTouchComponent::*)(BaseEntity*)>(&BaseTouchComponent::RemoveTouch),this,std::placeholders::_1)
			)
		)
	));
}
void BaseTouchComponent::Contact(const PhysContact &contact)
{
	m_contactReport.emplace_back(contact);
}

bool BaseTouchComponent::CanTrigger(BaseEntity *ent,PhysObj *phys)
{
	auto evCanTriggerData = CECanTriggerData{ent,phys};
	if(BroadcastEvent(EVENT_CAN_TRIGGER,evCanTriggerData) == util::EventReply::Handled)
		return evCanTriggerData.canTrigger;
	if(
		(m_triggerFlags &TriggerFlags::Everything) != TriggerFlags::Everything &&
		(ent->IsPlayer() == false || (m_triggerFlags &TriggerFlags::Players) == TriggerFlags::None) &&
		(ent->IsNPC() == false || (m_triggerFlags &TriggerFlags::NPCs) == TriggerFlags::None) &&
		((ent->IsPlayer() == false && ent->IsNPC() == false) || ent->GetPhysicsComponent().expired() || ent->GetPhysicsComponent()->GetPhysicsObject() == nullptr || (m_triggerFlags &TriggerFlags::Physics) == TriggerFlags::None)
	)
		return false;
	return evCanTriggerData.canTrigger;
}
void BaseTouchComponent::OnStartTouch(BaseEntity *ent,PhysObj *phys)
{
	BroadcastEvent(EVENT_ON_START_TOUCH,CETouchData{ent,phys});

	auto *l = ent->GetNetworkState()->GetGameState()->GetLuaState();
	auto &entThis = GetEntity();
	auto *pIoComponent = static_cast<BaseIOComponent*>(entThis.FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("onstarttouch",ent);
}
void BaseTouchComponent::OnEndTouch(BaseEntity *ent,PhysObj *phys)
{
	BroadcastEvent(EVENT_ON_END_TOUCH,CETouchData{ent,phys});

	auto *l = ent->GetNetworkState()->GetGameState()->GetLuaState();
	auto &entThis = GetEntity();
	auto *pIoComponent = static_cast<BaseIOComponent*>(entThis.FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("onendtouch",ent);
}
void BaseTouchComponent::OnTrigger(BaseEntity *ent,PhysObj *phys)
{
	BroadcastEvent(EVENT_ON_TRIGGER,CETouchData{ent,phys});

	auto *l = ent->GetNetworkState()->GetGameState()->GetLuaState();
	auto &entThis = GetEntity();
	auto *pIoComponent = static_cast<BaseIOComponent*>(entThis.FindComponent("io").get());
	if(pIoComponent != nullptr)
		pIoComponent->TriggerOutput("trigger",ent);
}
void BaseTouchComponent::Trigger(BaseEntity *ent,PhysObj *phys) {OnTrigger(ent,phys);}

std::vector<PhysTouch>::iterator BaseTouchComponent::RemoveTouch(std::vector<PhysTouch>::iterator it)
{
	auto &touch = *it;
	auto *entRemove = touch.entity.get();
	if(entRemove != nullptr)
	{
		auto pPhysComponent = entRemove->GetPhysicsComponent();
		EndTouch(entRemove,pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr);
	}
	it = m_touching.erase(it);
	if(m_touching.empty())
	{
		auto *pIoComponent = static_cast<BaseIOComponent*>(GetEntity().FindComponent("io").get());
		if(pIoComponent != nullptr)
			pIoComponent->TriggerOutput("onendtouchall",entRemove);
	}
	return it;
}
void BaseTouchComponent::RemoveTouch(BaseEntity *entRemove)
{
	auto it = std::find_if(m_touching.begin(),m_touching.end(),[entRemove](const PhysTouch &touch) {
		return (touch.entity.get() == entRemove) ? true : false;
	});
	if(it == m_touching.end())
		return;
	RemoveTouch(it);
}
void BaseTouchComponent::StartTouch(PhysTouch &touch)
{
	BaseEntity *ent = touch.entity.get();
	if(ent == nullptr)
		return;
	auto &entThis = GetEntity();
	auto bFirst = m_touching.empty();
	m_touching.push_back(touch);
	auto pPhysComponent = ent->GetPhysicsComponent();
	PhysObj *phys = pPhysComponent.valid() ? pPhysComponent->GetPhysicsObject() : nullptr;
	auto hEnt = entThis.GetHandle();
	OnStartTouch(ent,phys);
	if(bFirst)
	{
		auto *pIoComponent = static_cast<BaseIOComponent*>(entThis.FindComponent("io").get());
		if(pIoComponent != nullptr)
			pIoComponent->TriggerOutput("onstarttouchall",ent);
	}
	if(!hEnt.IsValid())
		return;
	if(pPhysComponent.valid() && pPhysComponent->IsTrigger())
		Trigger(ent,phys);
}
void BaseTouchComponent::StartTouch(BaseEntity *ent,PhysObj *phys)
{
	if(!CanTrigger(ent,phys) || IsTouching(ent))
		return;
	PhysTouch touch = PhysTouch(
		ent,
		ent->CallOnRemove(
			FunctionCallback<void,BaseEntity*>::Create(
				std::bind(static_cast<void(BaseTouchComponent::*)(BaseEntity*)>(&BaseTouchComponent::RemoveTouch),this,std::placeholders::_1)
			)
		)
	);
	StartTouch(touch);
}
void BaseTouchComponent::SetTriggerFlags(TriggerFlags flags) {m_triggerFlags = flags;}
BaseTouchComponent::TriggerFlags BaseTouchComponent::GetTriggerFlags() const {return m_triggerFlags;}
bool BaseTouchComponent::IsTouching(BaseEntity *ent)
{
	for(int i=0;i<m_touching.size();i++)
	{
		PhysTouch &touch = m_touching[i];
		if(touch.entity.IsValid() && touch.entity.get() == ent)
			return true;
	}
	return false;
}
void BaseTouchComponent::EndTouch(BaseEntity *ent,PhysObj *phys)
{
	auto hEnt = GetEntity().GetHandle();
	OnEndTouch(ent,phys);
	if(!hEnt.IsValid())
		return;
	Trigger(ent,phys);
}
void BaseTouchComponent::GetTouchingEntities(std::vector<BaseEntity*> &touching)
{
	for(unsigned int i=0;i<m_touching.size();i++)
	{
		PhysTouch &touch = m_touching[i];
		if(touch.entity.IsValid())
			touching.push_back(touch.entity.get());
	}
}
const std::vector<PhysTouch> &BaseTouchComponent::GetTouchingInfo() const {return m_touching;}

////////////////

CECanTriggerData::CECanTriggerData(BaseEntity *ent,PhysObj *phys)
	: entity(ent),physObj(phys)
{}
uint32_t CECanTriggerData::GetReturnCount() {return 1u;}
void CECanTriggerData::HandleReturnValues(lua_State *l)
{
	if(Lua::IsSet(l,-1))
		canTrigger = Lua::CheckBool(l,-1);
}
void CECanTriggerData::PushArguments(lua_State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject()->push(l);
	else
		Lua::PushNil(l);
	if(physObj != nullptr)
		Lua::Push<PhysObjHandle>(l,physObj->GetHandle());
	else
		Lua::PushNil(l);
}

////////////////

CETouchData::CETouchData(BaseEntity *ent,PhysObj *phys)
	: entity(ent),physObj(phys)
{}
void CETouchData::PushArguments(lua_State *l)
{
	if(entity != nullptr)
		entity->GetLuaObject()->push(l);
	else
		Lua::PushNil(l);
	if(physObj != nullptr)
		Lua::Push<PhysObjHandle>(l,physObj->GetHandle());
	else
		Lua::PushNil(l);
}
