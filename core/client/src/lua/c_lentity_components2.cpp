/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/liquid/c_liquid_control_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_simulation_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_static_bvh_cache_component.hpp"
#include "pragma/entities/components/c_static_bvh_user_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
// --template-include-location

void RegisterLuaEntityComponents2(lua_State *l,luabind::module_ &entsMod)
{
	auto defLiquidControl = pragma::lua::create_entity_component_class<pragma::CLiquidControlComponent,pragma::BaseLiquidControlComponent>("LiquidControlComponent");
	entsMod[defLiquidControl];

	auto defLiquidSurfaceSimulation = pragma::lua::create_entity_component_class<pragma::CLiquidSurfaceSimulationComponent,pragma::BaseLiquidSurfaceSimulationComponent>("LiquidSurfaceSimulationComponent");
	entsMod[defLiquidSurfaceSimulation];

	auto defBvh = pragma::lua::create_entity_component_class<pragma::CBvhComponent,pragma::BaseBvhComponent>("BvhComponent");
	entsMod[defBvh];

	auto defStaticBvh = pragma::lua::create_entity_component_class<pragma::CStaticBvhCacheComponent,pragma::BaseStaticBvhCacheComponent>("StaticBvhCacheComponent");
	entsMod[defStaticBvh];

	auto defStaticBvhUser = pragma::lua::create_entity_component_class<pragma::CStaticBvhUserComponent,pragma::BaseStaticBvhUserComponent>("StaticBvhUserComponent");
	entsMod[defStaticBvhUser];
	// --template-component-register-location
}
