// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.triggers.teleport;

import :game;

using namespace pragma;

void STriggerTeleportComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void TriggerTeleport::Initialize()
{
	TriggerTouch::Initialize();
	AddComponent<STriggerTeleportComponent>();
}
