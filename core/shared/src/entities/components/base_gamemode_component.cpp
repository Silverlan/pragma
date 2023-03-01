/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_gamemode_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/game/gamemode/gamemodemanager.h"

using namespace pragma;

ComponentEventId BaseGamemodeComponent::EVENT_ON_PLAYER_DEATH = INVALID_COMPONENT_ID;
ComponentEventId BaseGamemodeComponent::EVENT_ON_PLAYER_SPAWNED = INVALID_COMPONENT_ID;
ComponentEventId BaseGamemodeComponent::EVENT_ON_PLAYER_DROPPED = INVALID_COMPONENT_ID;
ComponentEventId BaseGamemodeComponent::EVENT_ON_PLAYER_READY = INVALID_COMPONENT_ID;
ComponentEventId BaseGamemodeComponent::EVENT_ON_PLAYER_JOINED = INVALID_COMPONENT_ID;
ComponentEventId BaseGamemodeComponent::EVENT_ON_GAME_INITIALIZED = INVALID_COMPONENT_ID;
ComponentEventId BaseGamemodeComponent::EVENT_ON_MAP_INITIALIZED = INVALID_COMPONENT_ID;
ComponentEventId BaseGamemodeComponent::EVENT_ON_GAME_READY = INVALID_COMPONENT_ID;
void BaseGamemodeComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	EVENT_ON_PLAYER_DEATH = registerEvent("ON_PLAYER_DEATH", ComponentEventInfo::Type::Explicit);
	EVENT_ON_PLAYER_SPAWNED = registerEvent("ON_PLAYER_SPAWNED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_PLAYER_DROPPED = registerEvent("ON_PLAYER_DROPPED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_PLAYER_READY = registerEvent("ON_PLAYER_READY", ComponentEventInfo::Type::Explicit);
	EVENT_ON_PLAYER_JOINED = registerEvent("ON_PLAYER_JOINED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_GAME_INITIALIZED = registerEvent("ON_GAME_INITIALIZED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_MAP_INITIALIZED = registerEvent("ON_MAP_INITIALIZED", ComponentEventInfo::Type::Explicit);
	EVENT_ON_GAME_READY = registerEvent("ON_GAME_READY", ComponentEventInfo::Type::Explicit);
}
BaseGamemodeComponent::BaseGamemodeComponent(BaseEntity &ent) : BaseEntityComponent(ent) { ent.GetNetworkState()->GetGameState()->GetGamemodeComponents().push_back(this); }

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

void BaseGamemodeComponent::OnPlayerDeath(BasePlayerComponent &pl, DamageInfo *dmgInfo)
{
	CEPlayerDeath evData {pl, dmgInfo};
	BroadcastEvent(EVENT_ON_PLAYER_DEATH, evData);
}
void BaseGamemodeComponent::OnPlayerSpawned(BasePlayerComponent &pl)
{
	CEPlayerSpawned evData {pl};
	BroadcastEvent(EVENT_ON_PLAYER_SPAWNED, evData);
}
void BaseGamemodeComponent::OnPlayerDropped(BasePlayerComponent &pl, pragma::networking::DropReason reason)
{
	CEPlayerDropped evData {pl, reason};
	BroadcastEvent(EVENT_ON_PLAYER_DROPPED, evData);
}
void BaseGamemodeComponent::OnPlayerReady(BasePlayerComponent &pl)
{
	CEPlayerReady evData {pl};
	BroadcastEvent(EVENT_ON_PLAYER_READY, evData);
}
void BaseGamemodeComponent::OnPlayerJoined(BasePlayerComponent &pl)
{
	CEPlayerJoined evData {pl};
	BroadcastEvent(EVENT_ON_PLAYER_JOINED, evData);
}
void BaseGamemodeComponent::OnGameInitialized() { BroadcastEvent(EVENT_ON_GAME_INITIALIZED); }
void BaseGamemodeComponent::OnMapInitialized() { BroadcastEvent(EVENT_ON_MAP_INITIALIZED); }
void BaseGamemodeComponent::OnGameReady() { BroadcastEvent(EVENT_ON_GAME_READY); }

GameModeInfo *BaseGamemodeComponent::GetGameModeInfo() { return GetEntity().GetNetworkState()->GetGameState()->GetGameMode(); }
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

CEPlayerDeath::CEPlayerDeath(BasePlayerComponent &pl, DamageInfo *dmgInfo) : player {pl}, dmgInfo {dmgInfo} {}
void CEPlayerDeath::PushArguments(lua_State *l)
{
	player.PushLuaObject(l);
	if(dmgInfo)
		Lua::Push<DamageInfo *>(l, dmgInfo);
}

CEPlayerDropped::CEPlayerDropped(BasePlayerComponent &pl, pragma::networking::DropReason reason) : player {pl}, reason {reason} {}
void CEPlayerDropped::PushArguments(lua_State *l)
{
	player.PushLuaObject(l);
	Lua::PushInt(l, umath::to_integral(reason));
}

CEPlayerSpawned::CEPlayerSpawned(BasePlayerComponent &pl) : player {pl} {}
void CEPlayerSpawned::PushArguments(lua_State *l) { player.PushLuaObject(l); }
