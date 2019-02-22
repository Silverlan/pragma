#include "stdafx_server.h"
#include "pragma/entities/components/s_generic_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

luabind::object SGenericComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SGenericComponentHandleWrapper>(l);}
