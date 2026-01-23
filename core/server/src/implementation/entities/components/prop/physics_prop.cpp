// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.prop.physics;

using namespace pragma;

void SPropPhysicsComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PropPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPropPhysicsComponent>();
}
