// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_server.h"
#include "luasystem.h"


import pragma.server.entities.components;

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
