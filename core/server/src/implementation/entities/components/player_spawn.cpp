// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.player_spawn;

import :entities.components;

using namespace pragma;

void SPlayerSpawnComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void GamePlayerSpawn::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPlayerSpawnComponent>();
	AddComponent<STransformComponent>();
}
