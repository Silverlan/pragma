// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/c_static_bvh_user_component.hpp"

import pragma.client.entities.components;

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
