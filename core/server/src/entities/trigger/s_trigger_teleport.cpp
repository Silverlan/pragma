#include "stdafx_server.h"
#include "pragma/entities/trigger/s_trigger_teleport.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_teleport,TriggerTeleport);

extern DLLSERVER SGame *s_game;

luabind::object STriggerTeleportComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<STriggerTeleportComponentHandleWrapper>(l);}

void TriggerTeleport::Initialize()
{
	TriggerTouch::Initialize();
	AddComponent<STriggerTeleportComponent>();
}
