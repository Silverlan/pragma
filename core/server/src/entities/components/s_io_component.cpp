#include "stdafx_server.h"
#include "pragma/entities/components/s_io_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

void SIOComponent::Initialize()
{
	BaseIOComponent::Initialize();
}
luabind::object SIOComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SIOComponentHandleWrapper>(l);}
