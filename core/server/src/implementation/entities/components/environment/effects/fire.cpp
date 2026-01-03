// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.effects.fire;

using namespace pragma;

void SFireComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvFire::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFireComponent>();
}
