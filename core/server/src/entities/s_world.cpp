#include "stdafx_server.h"
#include "pragma/entities/world.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/physics/collisiontypes.h"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(world,World);

luabind::object SWorldComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SWorldComponentHandleWrapper>(l);}

void World::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SWorldComponent>();
}
