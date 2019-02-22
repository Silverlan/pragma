#include "stdafx_client.h"
#include "pragma/entities/components/c_io_component.hpp"

using namespace pragma;

luabind::object CIOComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CIOComponentHandleWrapper>(l);}
