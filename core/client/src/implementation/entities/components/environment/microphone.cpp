// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.env_microphone;
using namespace pragma;

void CMicrophoneComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CEnvMicrophone::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CMicrophoneComponent>();
}
