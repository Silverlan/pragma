// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.shared;

import :entities.components.damageable;

using namespace pragma;

ComponentEventId DamageableComponent::EVENT_ON_TAKE_DAMAGE = pragma::INVALID_COMPONENT_ID;
void DamageableComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { EVENT_ON_TAKE_DAMAGE = registerEvent("ON_TAKE_DAMAGE", ComponentEventInfo::Type::Broadcast); }
DamageableComponent::DamageableComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void DamageableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("health");
}

void DamageableComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void DamageableComponent::OnTakeDamage(DamageInfo &info) {}

void DamageableComponent::TakeDamage(DamageInfo &info)
{
	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	game->CallCallbacks<void, BaseEntity *, std::reference_wrapper<DamageInfo>>("OnEntityTakeDamage", &ent, std::ref<DamageInfo>(info));
	OnTakeDamage(info);

	CEOnTakeDamage takeDmgInfo {info};
	BroadcastEvent(EVENT_ON_TAKE_DAMAGE, takeDmgInfo);
}

//////////////

CEOnTakeDamage::CEOnTakeDamage(DamageInfo &damageInfo) : damageInfo {damageInfo} {}
void CEOnTakeDamage::PushArguments(lua_State *l) { Lua::Push<DamageInfo *>(l, &damageInfo); }
