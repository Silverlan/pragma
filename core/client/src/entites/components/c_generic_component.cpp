#include "stdafx_client.h"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

luabind::object CGenericComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CGenericComponentHandleWrapper>(l);}
