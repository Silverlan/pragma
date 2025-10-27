// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/lua/core.hpp"

#include <string>

module pragma.shared;

import :entities.components.map;

using namespace pragma;

MapComponent::MapComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}

void MapComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	GetEntity().AddComponent("name");
}

void MapComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void MapComponent::SetMapIndex(unsigned int idx) { m_mapIndex = idx; }
unsigned int MapComponent::GetMapIndex() const { return m_mapIndex; }
