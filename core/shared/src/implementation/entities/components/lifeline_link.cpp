// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;


#include "pragma/lua/core.hpp"


module pragma.shared;

import :entities.components.lifeline_link;

using namespace pragma;

LifelineLinkComponent::LifelineLinkComponent(pragma::ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
void LifelineLinkComponent::Initialize() { BaseEntityComponent::Initialize(); }
void LifelineLinkComponent::InitializeLuaObject(lua_State *l) { pragma::BaseLuaHandle::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void LifelineLinkComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();
	for(auto it = m_entsRemove.begin(); it != m_entsRemove.end(); ++it) {
		auto &hEnt = *it;
		if(hEnt.IsValid())
			hEnt->Remove();
	}
}

void LifelineLinkComponent::RemoveEntityOnRemoval(pragma::ecs::BaseEntity *ent, Bool bRemove) { RemoveEntityOnRemoval(ent->GetHandle(), bRemove); }
void LifelineLinkComponent::RemoveEntityOnRemoval(const EntityHandle &hEnt, Bool bRemove)
{
	if(!hEnt.valid())
		return;
	auto *ent = hEnt.get();
	auto it = std::find_if(m_entsRemove.begin(), m_entsRemove.end(), [ent](EntityHandle &hOther) { return (hOther.valid() && hOther.get() == ent) ? true : false; });
	if(bRemove == true) {
		if(it == m_entsRemove.end())
			m_entsRemove.push_back(hEnt);
		return;
	}
	else if(it == m_entsRemove.end())
		return;
	m_entsRemove.erase(it);
}
