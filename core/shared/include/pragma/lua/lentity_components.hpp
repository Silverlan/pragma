#ifndef __LENTITY_COMPONENTS_HPP__
#define __LENTITY_COMPONENTS_HPP__

#include "pragma/networkdefinitions.h"

struct lua_State;
namespace luabind {class module_;};
struct BaseSoundEmitterComponentHandleWrapper;
namespace Lua
{
	DLLNETWORK void register_entity_component(lua_State *l,luabind::module_ &module);
	DLLNETWORK void register_gravity_component(luabind::module_ &module);
};

#endif
