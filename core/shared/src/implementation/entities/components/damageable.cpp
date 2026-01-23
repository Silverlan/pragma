// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.damageable;

using namespace pragma;

void DamageableComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { damageableComponent::EVENT_ON_TAKE_DAMAGE = registerEvent("ON_TAKE_DAMAGE", ComponentEventInfo::Type::Broadcast); }
DamageableComponent::DamageableComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void DamageableComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent("health");
}

void DamageableComponent::InitializeLuaObject(lua::State *l) { BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void DamageableComponent::OnTakeDamage(game::DamageInfo &info) {}

void DamageableComponent::TakeDamage(game::DamageInfo &info)
{
	auto &ent = GetEntity();
	auto *state = ent.GetNetworkState();
	auto *game = state->GetGameState();
	game->CallCallbacks<void, ecs::BaseEntity *, std::reference_wrapper<game::DamageInfo>>("OnEntityTakeDamage", &ent, std::ref<game::DamageInfo>(info));
	OnTakeDamage(info);

	CEOnTakeDamage takeDmgInfo {info};
	BroadcastEvent(damageableComponent::EVENT_ON_TAKE_DAMAGE, takeDmgInfo);
}

//////////////

CEOnTakeDamage::CEOnTakeDamage(game::DamageInfo &damageInfo) : damageInfo {damageInfo} {}
void CEOnTakeDamage::PushArguments(lua::State *l) { Lua::Push<game::DamageInfo *>(l, &damageInfo); }
