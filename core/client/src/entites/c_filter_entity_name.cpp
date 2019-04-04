#include "stdafx_client.h"
#include "pragma/entities/c_filter_entity_name.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(filter_entity_name,CFilterEntityName);

extern CGame *s_game;

luabind::object CFilterNameComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CFilterNameComponentHandleWrapper>(l);}

void CFilterEntityName::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFilterNameComponent>();
}
