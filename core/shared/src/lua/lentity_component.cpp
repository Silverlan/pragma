/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/lentity_components.hpp"
#include "pragma/entities/components/base_actor_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/entities/entity_component_event.hpp"
#include "pragma/lua/lua_call.hpp"
#include "pragma/lua/lua_component_event.hpp"
#include "pragma/lua/converters/property_converter_t.hpp"
#include <sharedutils/datastream.h>
#include <udm.hpp>

extern DLLNETWORK Engine *engine;

void Game::RegisterLuaEntityComponent(luabind::class_<pragma::BaseEntityComponent> &def)
{
	// TODO: Remove this function
}
