#include "stdafx_server.h"
#include "pragma/entities/s_bot.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/ai/navsystem.h>
#include <pragma/model/animation/activities.h>
#include "pragma/ai/ai_schedule.h"
#include "pragma/ai/s_disposition.h"
#include "pragma/lua/s_lentity_handles.hpp"

using namespace pragma;

LINK_ENTITY_TO_CLASS(bot,Bot);

luabind::object SBotComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SBotComponentHandleWrapper>(l);}

void Bot::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SBotComponent>();
}
