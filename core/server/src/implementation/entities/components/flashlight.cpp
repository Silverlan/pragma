// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"

module pragma.server.entities.components.flashlight;

using namespace pragma;

void SFlashlightComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void Flashlight::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFlashlightComponent>();
}
