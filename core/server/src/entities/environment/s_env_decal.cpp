#include "stdafx_server.h"
#include "pragma/entities/environment/s_env_decal.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/environment/env_decal.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_decal,EnvDecal);

luabind::object SDecalComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SDecalComponentHandleWrapper>(l);}

void SDecalComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = static_cast<SBaseEntity&>(GetEntity());
	ent.SetSynchronized(false);
}

void EnvDecal::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SDecalComponent>();
}
