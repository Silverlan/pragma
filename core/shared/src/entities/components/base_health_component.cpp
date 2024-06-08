/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_health_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

using namespace pragma;

ComponentEventId BaseHealthComponent::EVENT_ON_TAKEN_DAMAGE = pragma::INVALID_COMPONENT_ID;
ComponentEventId BaseHealthComponent::EVENT_ON_HEALTH_CHANGED = pragma::INVALID_COMPONENT_ID;
void BaseHealthComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_TAKEN_DAMAGE = registerEvent("ON_TAKEN_DAMAGE", ComponentEventInfo::Type::Broadcast);
	EVENT_ON_HEALTH_CHANGED = registerEvent("ON_HEALTH_CHANGED", ComponentEventInfo::Type::Broadcast);
}
void BaseHealthComponent::RegisterMembers(pragma::EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
{
	using T = BaseHealthComponent;

	using THealth = uint16_t;
	{
		auto memberInfo = create_component_member_info<T, THealth, static_cast<void (T::*)(THealth)>(&T::SetHealth), static_cast<THealth (T::*)() const>(&T::GetHealth)>("health", 0);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}

	{
		auto memberInfo = create_component_member_info<T, THealth, static_cast<void (T::*)(THealth)>(&T::SetMaxHealth), static_cast<THealth (T::*)() const>(&T::GetMaxHealth)>("maxHealth", 0);
		memberInfo.SetMin(0.f);
		registerMember(std::move(memberInfo));
	}
}
BaseHealthComponent::BaseHealthComponent(BaseEntity &ent) : BaseEntityComponent(ent), m_health(util::UInt16Property::Create(0)), m_maxHealth(util::UInt16Property::Create(0)) {}
void BaseHealthComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(ustring::compare<std::string>(kvData.key, "health", false))
			*m_health = util::to_int(kvData.value);
		else if(ustring::compare<std::string>(kvData.key, "max_health", false))
			*m_maxHealth = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(BaseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(ustring::compare<std::string>(inputData.input, "sethealth", false))
			*m_health = util::to_int(inputData.data);
		else if(ustring::compare<std::string>(inputData.input, "setmaxhealth", false))
			*m_maxHealth = util::to_int(inputData.data);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("io");
	ent.AddComponent("damageable");
}

util::EventReply BaseHealthComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == DamageableComponent::EVENT_ON_TAKE_DAMAGE)
		OnTakeDamage(static_cast<CEOnTakeDamage &>(evData).damageInfo);
	return util::EventReply::Unhandled;
}

void BaseHealthComponent::OnTakeDamage(DamageInfo &info)
{
	auto health = GetHealth();
	unsigned short dmg = info.GetDamage();
	if(dmg >= *m_health)
		SetHealth(0);
	else
		SetHealth(*m_health - dmg);

	auto newHealth = GetHealth();
	CEOnTakenDamage takeDmgInfo {info, health, newHealth};
	BroadcastEvent(EVENT_ON_TAKEN_DAMAGE, takeDmgInfo);
}

const util::PUInt16Property &BaseHealthComponent::GetHealthProperty() const { return m_health; }
const util::PUInt16Property &BaseHealthComponent::GetMaxHealthProperty() const { return m_maxHealth; }
uint16_t BaseHealthComponent::GetHealth() const { return *m_health; }
uint16_t BaseHealthComponent::GetMaxHealth() const { return *m_maxHealth; }
void BaseHealthComponent::SetHealth(uint16_t health)
{
    if(*m_health == health) //had to switch order of compared values, otherwise we got an error of ambiguity of operator==
		return;
	unsigned short old = *m_health;
	*m_health = health;
	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	game->CallCallbacks<void, BaseEntity *, uint16_t, uint16_t>("OnEntityHealthChanged", &ent, old, *m_health);

	CEOnHealthChanged evData {old, *m_health};
	BroadcastEvent(EVENT_ON_HEALTH_CHANGED, evData);
}
void BaseHealthComponent::SetMaxHealth(uint16_t maxHealth) { *m_maxHealth = maxHealth; }

void BaseHealthComponent::Save(udm::LinkedPropertyWrapperArg udm)
{
	BaseEntityComponent::Save(udm);
	udm["health"] = **m_health;
	udm["maxHealth"] = **m_maxHealth;
}
void BaseHealthComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version)
{
	BaseEntityComponent::Load(udm, version);
	uint16_t health = 0;
	udm["health"](health);
	uint16_t maxHealth = 0;
	udm["maxHealth"](maxHealth);
	SetMaxHealth(maxHealth);
	SetHealth(health);
}

//////////////

CEOnTakenDamage::CEOnTakenDamage(DamageInfo &damageInfo, uint16_t oldHealth, uint16_t newHealth) : damageInfo {damageInfo}, oldHealth(oldHealth), newHealth(newHealth) {}
void CEOnTakenDamage::PushArguments(lua_State *l)
{
	Lua::Push<DamageInfo *>(l, &damageInfo);
	Lua::PushInt(l, oldHealth);
	Lua::PushInt(l, newHealth);
}

//////////////

CEOnHealthChanged::CEOnHealthChanged(uint16_t oldHealth, uint16_t newHealth) : oldHealth(oldHealth), newHealth(newHealth) {}
void CEOnHealthChanged::PushArguments(lua_State *l)
{
	Lua::PushInt(l, oldHealth);
	Lua::PushInt(l, newHealth);
}
