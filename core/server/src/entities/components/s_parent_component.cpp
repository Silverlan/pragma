#include "stdafx_server.h"
#include "pragma/entities/components/s_parent_component.hpp"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

void SParentComponent::Initialize()
{
	BaseParentComponent::Initialize();
}
luabind::object SParentComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SParentComponentHandleWrapper>(l);}
