// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_health;

using namespace pragma;

void BaseHealthComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseHealthComponent::EVENT_ON_TAKEN_DAMAGE = registerEvent("ON_TAKEN_DAMAGE", ComponentEventInfo::Type::Broadcast);
	baseHealthComponent::EVENT_ON_HEALTH_CHANGED = registerEvent("ON_HEALTH_CHANGED", ComponentEventInfo::Type::Broadcast);
}
void BaseHealthComponent::RegisterMembers(EntityComponentManager &componentManager, TRegisterComponentMember registerMember)
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
BaseHealthComponent::BaseHealthComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent), m_health(util::UInt16Property::Create(0)), m_maxHealth(util::UInt16Property::Create(0)) {}
void BaseHealthComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(ecs::baseEntity::EVENT_HANDLE_KEY_VALUE, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData &>(evData.get());
		if(pragma::string::compare<std::string>(kvData.key, "health", false))
			*m_health = util::to_int(kvData.value);
		else if(pragma::string::compare<std::string>(kvData.key, "max_health", false))
			*m_maxHealth = util::to_int(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});
	BindEvent(baseIOComponent::EVENT_HANDLE_INPUT, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto &inputData = static_cast<CEInputData &>(evData.get());
		if(pragma::string::compare<std::string>(inputData.input, "sethealth", false))
			*m_health = util::to_int(inputData.data);
		else if(pragma::string::compare<std::string>(inputData.input, "setmaxhealth", false))
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
	if(eventId == damageableComponent::EVENT_ON_TAKE_DAMAGE)
		OnTakeDamage(static_cast<CEOnTakeDamage &>(evData).damageInfo);
	return util::EventReply::Unhandled;
}

void BaseHealthComponent::OnTakeDamage(game::DamageInfo &info)
{
	auto health = GetHealth();
	unsigned short dmg = info.GetDamage();
	if(dmg >= *m_health)
		SetHealth(0);
	else
		SetHealth(*m_health - dmg);

	auto newHealth = GetHealth();
	CEOnTakenDamage takeDmgInfo {info, health, newHealth};
	BroadcastEvent(baseHealthComponent::EVENT_ON_TAKEN_DAMAGE, takeDmgInfo);
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
	game->CallCallbacks<void, ecs::BaseEntity *, uint16_t, uint16_t>("OnEntityHealthChanged", &ent, old, *m_health);

	CEOnHealthChanged evData {old, *m_health};
	BroadcastEvent(baseHealthComponent::EVENT_ON_HEALTH_CHANGED, evData);
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

CEOnTakenDamage::CEOnTakenDamage(game::DamageInfo &damageInfo, uint16_t oldHealth, uint16_t newHealth) : damageInfo {damageInfo}, oldHealth(oldHealth), newHealth(newHealth) {}
void CEOnTakenDamage::PushArguments(lua::State *l)
{
	Lua::Push<game::DamageInfo *>(l, &damageInfo);
	Lua::PushInt(l, oldHealth);
	Lua::PushInt(l, newHealth);
}

//////////////

CEOnHealthChanged::CEOnHealthChanged(uint16_t oldHealth, uint16_t newHealth) : oldHealth(oldHealth), newHealth(newHealth) {}
void CEOnHealthChanged::PushArguments(lua::State *l)
{
	Lua::PushInt(l, oldHealth);
	Lua::PushInt(l, newHealth);
}
