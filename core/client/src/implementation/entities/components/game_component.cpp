// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/lua/luaapi.h"

#include "stdafx_client.h"

module pragma.client;


import :entities.components.game;
import :entities.components.input;
import :entities.components.observer;

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
