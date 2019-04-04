#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_camera.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <pragma/networking/nwm_util.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_camera,EnvCamera);

luabind::object SCameraComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SCameraComponentHandleWrapper>(l);}

void EnvCamera::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SCameraComponent>();
}
