// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/core.hpp"

module pragma.server;
import :entities.components.triggers.hurt;

using namespace pragma;

void STriggerHurtComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerHurt::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerHurtComponent>();
}
