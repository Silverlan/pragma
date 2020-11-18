/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/func/c_funcbutton.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(func_button,CFuncButton);

void CButtonComponent::Initialize()
{
	BaseFuncButtonComponent::Initialize();
	auto pRenderComponent = static_cast<CBaseEntity&>(GetEntity()).GetRenderComponent();
	if(pRenderComponent)
		pRenderComponent->SetRenderMode(RenderMode::World);
}
luabind::object CButtonComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CButtonComponentHandleWrapper>(l);}

//////////

void CFuncButton::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CButtonComponent>();
}
