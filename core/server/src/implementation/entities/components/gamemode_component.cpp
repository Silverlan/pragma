// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.gamemode;

using namespace pragma;

void SGamemodeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

////////////

void SGamemode::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SGamemodeComponent>();
}
