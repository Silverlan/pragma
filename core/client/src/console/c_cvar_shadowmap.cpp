/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <wgui/types/wirect.h>
#include "pragma/gui/mainmenu/wimainmenu.h"
#include "pragma/game/c_game_createguielement.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/rendering/c_cubemapside.h"
#include "pragma/gui/debug/widebugshadowmap.hpp"
#include <pragma/console/sh_cmd.h>
#include <image/prosper_render_target.hpp>
#include <wgui/types/wiroot.h>


extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;
static WIHandle hGUIShadowmap;
static int numShadowmapTargets = 0;
static int shadowmapTargetIdx = -1;
static int shadowmapWidth;
static int shadowmapHeight;
static CallbackHandle cbRenderShadowMap;
static CallbackHandle cbReleaseShadowMap;
static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
static bool get_shadow_map(NetworkState *nw, std::vector<std::string> &argv, pragma::CLightComponent **light, pragma::CLightComponent::ShadowMapType smType)
{
	if(argv.empty())
		return false;
	auto ents = command::find_named_targets(nw, argv.front());
	if(ents.empty())
		return false;
	auto *ent = static_cast<CBaseEntity *>(ents.front());
	auto *pLightComponent = static_cast<pragma::CLightComponent *>(ent->FindComponent("light").get());
	if(pLightComponent == nullptr) {
		Con::cwar << "Entity '" << ent->GetClass() << "'(" << argv.front() << ") is not a light!" << Con::endl;
		return false;
	}
	*light = pLightComponent;
	if(pLightComponent->GetLight() == nullptr) {
		Con::cwar << "Entity '" << ent->GetClass() << "'(" << argv.front() << ") has no light attached!" << Con::endl;
		return false;
	}
	auto hShadowmap = (*light)->GetShadowMap(smType);
	if(hShadowmap.expired() && (*light)->GetEntity().HasComponent<pragma::CShadowCSMComponent>() == false) {
		Con::cwar << "Invalid shadowmap for this entity!" << Con::endl;
		return false;
	}
	return true;
}

void CMD_debug_light_shadowmap(NetworkState *nw, pragma::BasePlayerComponent *, std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	const std::string name = "debug_shadowmap";
	auto *pEl = pRoot->FindDescendantByName(name);
	if(pEl != nullptr)
		pEl->Remove();

	//auto smType = pragma::CLightComponent::ShadowMapType::Static;
	//if(argv.size() > 1 && util::to_boolean(argv.at(1)))
	//	smType = pragma::CLightComponent::ShadowMapType::Dynamic;
	auto smType = pragma::CLightComponent::ShadowMapType::Dynamic;

	pragma::CLightComponent *light;
	if(get_shadow_map(nw, argv, &light, smType) == false)
		return;
	if(c_game == nullptr || argv.empty() || pRoot == nullptr)
		return;
	auto *pElSm = wgui.Create<WIDebugShadowMap>();
	if(pElSm == nullptr)
		return;
	auto size = 256u;
	pElSm->SetZPos(2000);
	pElSm->SetLightSource(*light);
	pElSm->SetShadowMapSize(size, size);
	pElSm->SetShadowMapType(smType);
	pElSm->SetName(name);
	pElSm->Update();
}
static void CVAR_CALLBACK_cl_render_shadow_pssm_split_count(NetworkState *state, const ConVar &, int, int)
{
	if(shadowmapTargetIdx == -1)
		return;
	std::vector<std::string> argv = {std::to_string(shadowmapTargetIdx), std::to_string(shadowmapWidth), std::to_string(shadowmapHeight)};
	CMD_debug_light_shadowmap(state, nullptr, argv);
}
REGISTER_CONVAR_CALLBACK_CL(cl_render_shadow_pssm_split_count, CVAR_CALLBACK_cl_render_shadow_pssm_split_count);
