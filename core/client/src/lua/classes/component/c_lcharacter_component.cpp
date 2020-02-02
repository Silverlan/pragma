#include "stdafx_client.h"
#include "pragma/lua/classes/ldef_entity.h"
#include "luasystem.h"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/entities/components/c_character_component.hpp"
#include <pragma/physics/raytraces.h>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/entities/components/base_actor_component.hpp>

void Lua::register_cl_character_component(lua_State *l,luabind::module_ &module)
{
	auto def = luabind::class_<CCharacterHandle,BaseEntityComponentHandle>("CharacterComponent");
	Lua::register_base_character_component_methods<luabind::class_<CCharacterHandle,BaseEntityComponentHandle>,CCharacterHandle>(l,def);
	module[def];
}
