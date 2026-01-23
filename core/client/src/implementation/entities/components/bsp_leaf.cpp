// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.bsp_leaf;
import :engine;

using namespace pragma;

void CBSPLeafComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CBSPLeafComponent::SetLeaves(const std::vector<uint16_t> &leaves)
{
	for(auto idx : leaves)
		m_leaves.insert(idx);
}
bool CBSPLeafComponent::GetLeafVisibility(uint16_t leafIdx) const { return m_leaves.find(leafIdx) != m_leaves.end(); }
