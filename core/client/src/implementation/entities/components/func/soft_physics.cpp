// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.func_soft_physics;
using namespace pragma;

void CFuncSoftPhysicsComponent::Initialize()
{
	BaseFuncSoftPhysicsComponent::Initialize();
	GetEntity().AddComponent("func_physics");
}
void CFuncSoftPhysicsComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

///////////

void CFuncSoftPhysics::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFuncSoftPhysicsComponent>();
}
