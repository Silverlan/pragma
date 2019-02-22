#include "stdafx_server.h"
#include "pragma/entities/filter/s_filter_entity_name.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(filter_entity_name,FilterEntityName);

extern SGame *s_game;

luabind::object SFilterNameComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFilterNameComponentHandleWrapper>(l);}

void FilterEntityName::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFilterNameComponent>();
}
