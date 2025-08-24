// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>

module pragma.client.entities.components;

import :bsp_leaf;

extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CBSPLeafComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CBSPLeafComponent::SetLeaves(const std::vector<uint16_t> &leaves)
{
	for(auto idx : leaves)
		m_leaves.insert(idx);
}
bool CBSPLeafComponent::GetLeafVisibility(uint16_t leafIdx) const { return m_leaves.find(leafIdx) != m_leaves.end(); }
