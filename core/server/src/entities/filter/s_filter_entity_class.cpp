#include "stdafx_server.h"
#include "pragma/entities/filter/s_filter_entity_class.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(filter_entity_class,FilterEntityClass);

extern SGame *s_game;

luabind::object SFilterClassComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFilterClassComponentHandleWrapper>(l);}

void FilterEntityClass::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFilterClassComponent>();
}
