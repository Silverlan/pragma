#include "stdafx_server.h"
#include "pragma/entities/environment/effects/s_env_fire.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/game/damageinfo.h>
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_fire,EnvFire);

luabind::object SFireComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SFireComponentHandleWrapper>(l);}

void EnvFire::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SFireComponent>();
}
