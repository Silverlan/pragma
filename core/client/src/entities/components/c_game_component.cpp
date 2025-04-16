/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_game_component.hpp"
#include "pragma/entities/components/c_input_component.hpp"
#include "pragma/entities/components/c_observer_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/entity_component_system_t.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>

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
