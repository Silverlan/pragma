/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/parentinfo.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/entities/components/logic_component.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(viewbody,CViewBody);

extern ClientState *client;
extern CGame *c_game;

void CViewBodyComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(CRenderComponent::EVENT_SHOULD_DRAW,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &shouldDrawData = static_cast<CEShouldDraw&>(evData.get());
		auto *pl = c_game->GetLocalPlayer();
		if(pl == nullptr || pl->GetObserverMode() != OBSERVERMODE::FIRSTPERSON)
		{
			shouldDrawData.shouldDraw = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	ent.AddComponent<CTransformComponent>();
	ent.AddComponent<CModelComponent>();
	ent.AddComponent<LogicComponent>(); // Logic component is needed for animations
	auto pRenderComponent = ent.AddComponent<CRenderComponent>();
	if(pRenderComponent.valid())
	{
		pRenderComponent->SetRenderMode(RenderMode::View);
		pRenderComponent->SetCastShadows(false);
	}
	ent.AddComponent<CAnimatedComponent>();
}
void CViewBodyComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}

//////////////

void CViewBody::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CViewBodyComponent>();
}
