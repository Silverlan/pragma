#include "stdafx_server.h"
#include "pragma/entities/game_player_spawn.h"
#include "pragma/entities/s_entityfactories.h"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(game_player_spawn,GamePlayerSpawn);

luabind::object SPlayerSpawnComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SPlayerSpawnComponentHandleWrapper>(l);}

void GamePlayerSpawn::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPlayerSpawnComponent>();
	AddComponent<STransformComponent>();
}
