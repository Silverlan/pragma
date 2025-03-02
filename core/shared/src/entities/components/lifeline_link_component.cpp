/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2023 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/lifeline_link_component.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/component_member_flags.hpp"

using namespace pragma;

LifelineLinkComponent::LifelineLinkComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
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

void LifelineLinkComponent::RemoveEntityOnRemoval(BaseEntity *ent, Bool bRemove) { RemoveEntityOnRemoval(ent->GetHandle(), bRemove); }
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
