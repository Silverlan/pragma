// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/entities/components/c_game_component.hpp"
#include "pragma/entities/components/c_input_component.hpp"
#include "pragma/entities/components/c_observer_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include "pragma/entities/c_baseentity.h"

using namespace pragma;

LINK_ENTITY_TO_CLASS(game, CGameEntity);

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
