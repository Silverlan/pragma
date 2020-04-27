/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/c_env_microphone.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_microphone,CEnvMicrophone);

luabind::object CMicrophoneComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CMicrophoneComponentHandleWrapper>(l);}

void CEnvMicrophone::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CMicrophoneComponent>();
}
