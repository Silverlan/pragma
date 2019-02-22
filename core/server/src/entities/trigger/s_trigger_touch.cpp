#include "stdafx_server.h"
#include "pragma/entities/trigger/s_trigger_touch.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <pragma/physics/collisionmasks.h>
#include "pragma/entities/trigger/trigger_spawnflags.h"
#include <sharedutils/util_string.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_touch,TriggerTouch);

luabind::object STouchComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<STouchComponentHandleWrapper>(l);}

void TriggerTouch::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STouchComponent>();
}
