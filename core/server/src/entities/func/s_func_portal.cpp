#include "stdafx_server.h"
#include "pragma/entities/func/s_func_portal.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_portal,FuncPortal);

luabind::object SFuncPortalComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFuncPortalComponentHandleWrapper>(l);}

void FuncPortal::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFuncPortalComponent>();
}
