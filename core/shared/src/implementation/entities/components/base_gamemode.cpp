// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cassert>

module pragma.shared;

import :entities.components.base_gamemode;

using namespace pragma;

void BaseGamemodeComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseGamemodeComponent::EVENT_ON_PLAYER_DEATH = registerEvent("ON_PLAYER_DEATH", ComponentEventInfo::Type::Explicit);
	baseGamemodeComponent::EVENT_ON_PLAYER_SPAWNED = registerEvent("ON_PLAYER_SPAWNED", ComponentEventInfo::Type::Explicit);
	baseGamemodeComponent::EVENT_ON_PLAYER_DROPPED = registerEvent("ON_PLAYER_DROPPED", ComponentEventInfo::Type::Explicit);
	baseGamemodeComponent::EVENT_ON_PLAYER_READY = registerEvent("ON_PLAYER_READY", ComponentEventInfo::Type::Explicit);
	baseGamemodeComponent::EVENT_ON_PLAYER_JOINED = registerEvent("ON_PLAYER_JOINED", ComponentEventInfo::Type::Explicit);
	baseGamemodeComponent::EVENT_ON_GAME_INITIALIZED = registerEvent("ON_GAME_INITIALIZED", ComponentEventInfo::Type::Explicit);
	baseGamemodeComponent::EVENT_ON_MAP_INITIALIZED = registerEvent("ON_MAP_INITIALIZED", ComponentEventInfo::Type::Explicit);
	baseGamemodeComponent::EVENT_ON_GAME_READY = registerEvent("ON_GAME_READY", ComponentEventInfo::Type::Explicit);
}
BaseGamemodeComponent::BaseGamemodeComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) { ent.GetNetworkState()->GetGameState()->GetGamemodeComponents().push_back(this); }

void BaseGamemodeComponent::Initialize() { BaseEntityComponent::Initialize(); }

void BaseGamemodeComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();

	auto &gmComponents = GetEntity().GetNetworkState()->GetGameState()->GetGamemodeComponents();
	auto it = std::find(gmComponents.begin(), gmComponents.end(), this);
	assert(it != gmComponents.end());
	gmComponents.erase(it);
}

void BaseGamemodeComponent::Save(udm::LinkedPropertyWrapperArg udm) { BaseEntityComponent::Save(udm); }

void BaseGamemodeComponent::Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) { BaseEntityComponent::Load(udm, version); }

void BaseGamemodeComponent::OnPlayerDeath(BasePlayerComponent &pl, game::DamageInfo *dmgInfo)
{
	CEPlayerDeath evData {pl, dmgInfo};
	BroadcastEvent(baseGamemodeComponent::EVENT_ON_PLAYER_DEATH, evData);
}
void BaseGamemodeComponent::OnPlayerSpawned(BasePlayerComponent &pl)
{
	CEPlayerSpawned evData {pl};
	BroadcastEvent(baseGamemodeComponent::EVENT_ON_PLAYER_SPAWNED, evData);
}
void BaseGamemodeComponent::OnPlayerDropped(BasePlayerComponent &pl, networking::DropReason reason)
{
	CEPlayerDropped evData {pl, reason};
	BroadcastEvent(baseGamemodeComponent::EVENT_ON_PLAYER_DROPPED, evData);
}
void BaseGamemodeComponent::OnPlayerReady(BasePlayerComponent &pl)
{
	CEPlayerReady evData {pl};
	BroadcastEvent(baseGamemodeComponent::EVENT_ON_PLAYER_READY, evData);
}
void BaseGamemodeComponent::OnPlayerJoined(BasePlayerComponent &pl)
{
	CEPlayerJoined evData {pl};
	BroadcastEvent(baseGamemodeComponent::EVENT_ON_PLAYER_JOINED, evData);
}
void BaseGamemodeComponent::OnGameInitialized() { BroadcastEvent(baseGamemodeComponent::EVENT_ON_GAME_INITIALIZED); }
void BaseGamemodeComponent::OnMapInitialized() { BroadcastEvent(baseGamemodeComponent::EVENT_ON_MAP_INITIALIZED); }
void BaseGamemodeComponent::OnGameReady() { BroadcastEvent(baseGamemodeComponent::EVENT_ON_GAME_READY); }

game::GameModeInfo *BaseGamemodeComponent::GetGameModeInfo() { return GetEntity().GetNetworkState()->GetGameState()->GetGameMode(); }
static const std::string empty_string {};
const std::string &BaseGamemodeComponent::GetName() const
{
	auto *gmInfo = GetGameModeInfo();
	return gmInfo ? gmInfo->name : empty_string;
}
const std::string &BaseGamemodeComponent::GetIdentifier() const
{
	auto *gmInfo = GetGameModeInfo();
	return gmInfo ? gmInfo->id : empty_string;
}
const std::string &BaseGamemodeComponent::GetComponentName() const
{
	auto *gmInfo = GetGameModeInfo();
	return gmInfo ? gmInfo->component_name : empty_string;
}
const std::string &BaseGamemodeComponent::GetAuthor() const
{
	auto *gmInfo = GetGameModeInfo();
	return gmInfo ? gmInfo->author : empty_string;
}
util::Version BaseGamemodeComponent::GetGamemodeVersion() const
{
	auto *gmInfo = GetGameModeInfo();
	return gmInfo ? gmInfo->version : util::Version {};
}

CEPlayerDeath::CEPlayerDeath(BasePlayerComponent &pl, game::DamageInfo *dmgInfo) : player {pl}, dmgInfo {dmgInfo} {}
void CEPlayerDeath::PushArguments(lua::State *l)
{
	player.PushLuaObject(l);
	if(dmgInfo)
		Lua::Push<game::DamageInfo *>(l, dmgInfo);
}

CEPlayerDropped::CEPlayerDropped(BasePlayerComponent &pl, networking::DropReason reason) : player {pl}, reason {reason} {}
void CEPlayerDropped::PushArguments(lua::State *l)
{
	player.PushLuaObject(l);
	Lua::PushInt(l, math::to_integral(reason));
}

CEPlayerSpawned::CEPlayerSpawned(BasePlayerComponent &pl) : player {pl} {}
void CEPlayerSpawned::PushArguments(lua::State *l) { player.PushLuaObject(l); }
