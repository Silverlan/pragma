// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.filter.entity_name;

import :game;

using namespace pragma;

void SFilterNameComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FilterEntityName::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFilterNameComponent>();
}
