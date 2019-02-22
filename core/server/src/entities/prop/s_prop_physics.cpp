#include "stdafx_server.h"
#include "pragma/entities/prop/s_prop_physics.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include "pragma/physics/movetypes.h"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(prop_physics,PropPhysics);

luabind::object SPropPhysicsComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPropPhysicsComponentHandleWrapper>(l);}

void PropPhysics::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPropPhysicsComponent>();
}
