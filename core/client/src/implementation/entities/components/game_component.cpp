// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include "pragma/entities/c_baseentity.h"

module pragma.client.entities.components.game;

import pragma.client.entities.components.input;
import pragma.client.entities.components.observer;

using namespace pragma;

void CGameComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CGameComponent::UpdateFrame(CCameraComponent *cam)
{
	auto inputC = GetEntity().GetComponent<CInputComponent>();
	if(inputC.valid())
		inputC->UpdateMouseMovementDeltaValues();

	if(cam) {
		auto observerC = cam->GetEntity().GetComponent<CObserverComponent>();
		if(observerC.valid())
			observerC->UpdateCharacterViewOrientationFromMouseMovement();
	}
}

void CGameComponent::UpdateCamera(CCameraComponent *cam)
{
	if(cam) {
		auto observerC = cam->GetEntity().GetComponent<CObserverComponent>();
		if(observerC.valid())
			observerC->UpdateCameraPose();
	}
}

////////////

void CGameEntity::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CGameComponent>();
	AddComponent<CInputComponent>();
}
