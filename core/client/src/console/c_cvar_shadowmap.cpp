

#include "sharedutils/functioncallback.h"

// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include <image/prosper_render_target.hpp>

import pragma.client;

static WIHandle hGUIShadowmap;
static int numShadowmapTargets = 0;
static int shadowmapTargetIdx = -1;
static int shadowmapWidth;
static int shadowmapHeight;
static CallbackHandle cbRenderShadowMap;
static CallbackHandle cbReleaseShadowMap;
static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
static bool get_shadow_map(NetworkState *nw, std::vector<std::string> &argv, pragma::CLightComponent **light, pragma::rendering::ShadowMapType smType)
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
	auto hShadowmap = (*light)->GetShadowMap<pragma::CShadowComponent>(smType);
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

	//auto smType = pragma::rendering::ShadowMapType::Static;
	//if(argv.size() > 1 && util::to_boolean(argv.at(1)))
	//	smType = pragma::rendering::ShadowMapType::Dynamic;
	auto smType = pragma::rendering::ShadowMapType::Dynamic;

	pragma::CLightComponent *light;
	if(get_shadow_map(nw, argv, &light, smType) == false)
		return;
	if(pragma::get_cgame() == nullptr || argv.empty() || pRoot == nullptr)
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
namespace { auto _ = pragma::console::client::register_variable_listener<int>("cl_render_shadow_pssm_split_count",&CVAR_CALLBACK_cl_render_shadow_pssm_split_count); }
