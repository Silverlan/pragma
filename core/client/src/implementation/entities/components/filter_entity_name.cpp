// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.filter_entity_name;
using namespace pragma;

void CFilterNameComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CFilterEntityName::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFilterNameComponent>();
}
