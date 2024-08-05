/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_static_bvh_user_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

using namespace pragma;

void CStaticBvhUserComponent::InitializeLuaObject(lua_State *l) { return BaseStaticBvhUserComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CStaticBvhUserComponent::Initialize() { BaseStaticBvhUserComponent::Initialize(); }

void CStaticBvhUserComponent::OnEntityComponentAdded(BaseEntityComponent &component)
{
	BaseStaticBvhUserComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CBvhComponent)) {
		m_bvhComponent = static_cast<CBvhComponent *>(&component);
		UpdateBvhStatus();
	}
}
void CStaticBvhUserComponent::OnEntityComponentRemoved(BaseEntityComponent &component)
{
	BaseStaticBvhUserComponent::OnEntityComponentAdded(component);
	if(typeid(component) == typeid(CBvhComponent)) {
		m_bvhComponent = nullptr;
		UpdateBvhStatus();
	}
}
