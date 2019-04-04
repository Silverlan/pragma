#include "stdafx_server.h"
#include "pragma/entities/prop/s_prop_dynamic.hpp"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include "pragma/physics/movetypes.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(prop_dynamic,PropDynamic);

luabind::object SPropDynamicComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPropDynamicComponentHandleWrapper>(l);}

void PropDynamic::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPropDynamicComponent>();
}
