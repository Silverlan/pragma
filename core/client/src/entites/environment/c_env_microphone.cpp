#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/c_env_microphone.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_microphone,CEnvMicrophone);

luabind::object CMicrophoneComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CMicrophoneComponentHandleWrapper>(l);}

void CEnvMicrophone::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CMicrophoneComponent>();
}
