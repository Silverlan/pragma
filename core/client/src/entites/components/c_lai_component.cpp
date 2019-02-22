#include "stdafx_client.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "pragma/entities/components/c_ai_component.hpp"
#include "luasystem.h"
#include "pragma/entities/components/c_lentity_components.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/model/model.h>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>

void Lua::register_cl_ai_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<CAIHandle,BaseEntityComponentHandle>("AIComponent");
	Lua::register_base_ai_component_methods<luabind::class_<CAIHandle,BaseEntityComponentHandle>,CAIHandle>(l,def);
	module[def];
}
