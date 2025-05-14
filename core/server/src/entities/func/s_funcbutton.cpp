/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/entities/func/s_funcbutton.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/entities/components/usable_component.hpp>
#include <sharedutils/util_string.h>
#include <sharedutils/util.h>
#include <pragma/audio/alsound.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_button, FuncButton);

extern ServerState *server;
extern SGame *s_game;

void SButtonComponent::Initialize()
{
	BaseFuncButtonComponent::Initialize();

	BindEvent(UsableComponent::EVENT_CAN_USE, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &bCanUse = static_cast<CECanUseData &>(evData.get()).canUse;
		bCanUse = (m_tNextUse - s_game->CurTime()) <= 0.f;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent<UsableComponent>();
}

////////////

void SButtonComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void FuncButton::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SButtonComponent>();
}
