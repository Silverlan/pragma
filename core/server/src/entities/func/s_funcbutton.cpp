#include "stdafx_server.h"
#include "pragma/entities/func/s_funcbutton.h"
#include "pragma/entities/s_entityfactories.h"
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/audio/alsound.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/audio/alsound_type.h>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_button,FuncButton);

extern ServerState *server;
extern SGame *s_game;

void SButtonComponent::Initialize()
{
	BaseFuncButtonComponent::Initialize();

	BindEvent(UsableComponent::EVENT_CAN_USE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &bCanUse = static_cast<CECanUseData&>(evData.get()).canUse;
		bCanUse = (m_tNextUse -s_game->CurTime()) <= 0.f;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent<UsableComponent>();
}

////////////

luabind::object SButtonComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<SButtonComponentHandleWrapper>(l);}

void FuncButton::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SButtonComponent>();
}
