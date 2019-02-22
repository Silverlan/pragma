#include "stdafx_client.h"
#include "pragma/entities/components/c_parent_component.hpp"

using namespace pragma;

luabind::object CParentComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CParentComponentHandleWrapper>(l);}
void CParentComponent::OnRemove()
{
	BaseParentComponent::OnRemove();
}
