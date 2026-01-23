// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.triggers.touch;

using namespace pragma;

void STouchComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerTouch::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STouchComponent>();
}
