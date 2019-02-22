#include "stdafx_shared.h"
#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/entity_component_event.hpp"

using namespace pragma;

CEOnEntityComponentAdded::CEOnEntityComponentAdded(BaseEntityComponent &component)
	: component(component)
{}
void CEOnEntityComponentAdded::PushArguments(lua_State *l)
{
	component.PushLuaObject(l);
}

/////////////

void CEGenericComponentEvent::PushArguments(lua_State *l) {}
