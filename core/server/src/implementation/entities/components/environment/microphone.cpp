// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include "sharedutils/netpacket.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

module pragma.server.entities.components.microphone;

using namespace pragma;

void SMicrophoneComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void EnvMicrophone::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SMicrophoneComponent>();
}
