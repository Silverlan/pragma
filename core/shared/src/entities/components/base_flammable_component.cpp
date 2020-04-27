/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_flammable_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include <sharedutils/datastream.h>

using namespace pragma;

ComponentEventId BaseFlammableComponent::EVENT_ON_IGNITED = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseFlammableComponent::EVENT_ON_EXTINGUISHED = pragma::INVALID_COMPONENT_ID;
void BaseFlammableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager)
{
	EVENT_ON_IGNITED = componentManager.RegisterEvent("ON_IGNITED");
	EVENT_ON_EXTINGUISHED = componentManager.RegisterEvent("ON_EXTINGUISHED");
}
BaseFlammableComponent::BaseFlammableComponent(BaseEntity &ent)
	: BaseEntityComponent(ent),m_bIsOnFire(util::BoolProperty::Create(false)),
	m_bIgnitable(util::BoolProperty::Create(true))
{}
BaseFlammableComponent::~BaseFlammableComponent()
{
	Extinguish();
}
void BaseFlammableComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"flammable",false))
			*m_bIgnitable = util::to_boolean(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData&>(evData.get());
		if(ustring::compare(inputData.input,"setflammable",false))
			*m_bIgnitable = util::to_boolean(inputData.data);
		else if(ustring::compare(inputData.input,"ignite",false))
			Ignite(util::to_float(inputData.data));
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEventUnhandled(LogicComponent::EVENT_ON_TICK,[this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		OnThink(static_cast<CEOnTick&>(evData.get()).deltaTime);
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	m_netEvIgnite = SetupNetEvent("ignite");
	m_netEvExtinguish = SetupNetEvent("extinguish");
	m_netEvSetIgnitable = SetupNetEvent("set_ignitable");
}
void BaseFlammableComponent::OnThink(double dt)
{
	if(IsOnFire() && m_tExtinguishTime > 0.f)
	{
		auto &t = GetEntity().GetNetworkState()->GetGameState()->CurTime();
		if(t >= m_tExtinguishTime)
			Extinguish();
	}
}
util::EventReply BaseFlammableComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == pragma::SubmergibleComponent::EVENT_ON_WATER_SUBMERGED)
		Extinguish();
	return util::EventReply::Unhandled;
}
void BaseFlammableComponent::Save(DataStream &ds)
{
	BaseEntityComponent::Save(ds);
	ds->Write<bool>(*m_bIgnitable);
	ds->Write<bool>(*m_bIsOnFire);
	auto tCur = GetEntity().GetNetworkState()->GetGameState()->CurTime();
	auto tExtinguish = m_tExtinguishTime;
	if(tExtinguish != 0.f)
		tExtinguish -= tCur;
	ds->Write<float>(tExtinguish);
}
void BaseFlammableComponent::Load(DataStream &ds,uint32_t version)
{
	BaseEntityComponent::Load(ds,version);
	auto bIgnitable = ds->Read<bool>();
	SetIgnitable(bIgnitable);

	auto bOnFire = ds->Read<bool>();
	auto tIgnite = ds->Read<float>();
	if(bOnFire == true)
		Ignite(tIgnite); // TODO: Attacker, inflictor?
}
const util::PBoolProperty &BaseFlammableComponent::GetOnFireProperty() const {return m_bIsOnFire;}
const util::PBoolProperty &BaseFlammableComponent::GetIgnitableProperty() const {return m_bIgnitable;}
bool BaseFlammableComponent::IsOnFire() const {return *m_bIsOnFire;}
bool BaseFlammableComponent::IsIgnitable() const {return *m_bIgnitable;}
util::EventReply BaseFlammableComponent::Ignite(float duration,BaseEntity *attacker,BaseEntity *inflictor)
{
	auto &ent = GetEntity();
	auto pSubmergibleComponent = ent.GetComponent<pragma::SubmergibleComponent>();
	if(pSubmergibleComponent.valid() && pSubmergibleComponent->IsSubmerged() == true)
		return util::EventReply::Handled;
	*m_bIsOnFire = true;
	if(duration == 0.f)
		m_tExtinguishTime = 0.f;
	else
	{
		auto tNew = ent.GetNetworkState()->GetGameState()->CurTime() +duration;
		if(tNew > m_tExtinguishTime)
			m_tExtinguishTime = static_cast<float>(tNew);
	}
	CEOnIgnited igniteData {duration,attacker,inflictor};
	return BroadcastEvent(EVENT_ON_IGNITED,igniteData);

}
void BaseFlammableComponent::Extinguish()
{
	if(*m_bIsOnFire == false)
		return;
	*m_bIsOnFire = false;
	m_tExtinguishTime = 0.f;
	BroadcastEvent(EVENT_ON_EXTINGUISHED);
}
void BaseFlammableComponent::SetIgnitable(bool b)
{
	*m_bIgnitable = b;
	if(b == false)
		Extinguish();
}

////////////

CEOnIgnited::CEOnIgnited(float duration,BaseEntity *attacker,BaseEntity *inflictor)
	: duration{duration},attacker{attacker ? attacker->GetHandle() : EntityHandle{}},
	inflictor{inflictor ? inflictor->GetHandle() : EntityHandle{}}
{}
void CEOnIgnited::PushArguments(lua_State *l)
{
	Lua::PushNumber(l,duration);

	if(attacker.IsValid())
		attacker->GetLuaObject()->push(l);
	else
		Lua::PushNil(l);

	if(inflictor.IsValid())
		inflictor->GetLuaObject()->push(l);
	else
		Lua::PushNil(l);
}
