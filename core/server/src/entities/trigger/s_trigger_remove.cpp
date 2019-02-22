#include "stdafx_server.h"
#include "pragma/entities/trigger/s_trigger_remove.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_remove,TriggerRemove);

luabind::object STriggerRemoveComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<STriggerRemoveComponentHandleWrapper>(l);}

void TriggerRemove::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerRemoveComponent>();
}
