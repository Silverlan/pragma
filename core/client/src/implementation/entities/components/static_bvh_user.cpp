// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.static_bvh_user;
import :engine;

using namespace pragma;

void CStaticBvhUserComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

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
