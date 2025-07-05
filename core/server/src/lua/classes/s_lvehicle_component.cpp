// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/entities/components/s_vehicle_component.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>

#include <pragma/lua/ostream_operator_alias.hpp>

namespace Lua {
	namespace Vehicle {
		namespace Server {

		};
	};
};

DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseVehicleComponent);

void Lua::register_sv_vehicle_component(lua_State *l, luabind::module_ &module)
{
	auto def = pragma::lua::create_entity_component_class<pragma::SVehicleComponent, pragma::BaseVehicleComponent>("VehicleComponent");
	module[def];
}
