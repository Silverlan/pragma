#include "stdafx_server.h"
#include "pragma/entities/s_skybox.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(skybox,Skybox);

luabind::object SSkyboxComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SSkyboxComponentHandleWrapper>(l);}

void Skybox::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SSkyboxComponent>();
}
