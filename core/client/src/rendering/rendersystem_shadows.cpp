/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;


void RenderSystem::RenderShadows(const util::DrawSceneInfo &drawSceneInfo,pragma::rendering::RasterizationRenderer &renderer,std::vector<pragma::CLightComponent*> &lights)
{
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CShadowManagerComponent>>();
	auto it = entIt.begin();
	if(it == entIt.end())
		return;
	auto shadowManagerC = it->GetComponent<pragma::CShadowManagerComponent>();
	if(lights.empty() == false)
	{
		for(auto *pLight : lights)
			shadowManagerC->GetRenderer().RenderShadows(drawSceneInfo.commandBuffer,*pLight);
	}

	// Directional light source is handled separately
	EntityIterator entItEnvLights {*c_game};
	entItEnvLights.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightDirectionalComponent>>();
	entItEnvLights.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightComponent>>();
	for(auto *ent : entItEnvLights)
	{
		auto toggleC = ent->GetComponent<pragma::CToggleComponent>();
		if(toggleC.valid() && toggleC->IsTurnedOn() == false)
			continue;
		renderer.UpdateCSMDescriptorSet(*ent->GetComponent<pragma::CLightDirectionalComponent>());
		shadowManagerC->GetRenderer().RenderShadows(drawSceneInfo.commandBuffer,*ent->GetComponent<pragma::CLightComponent>());
		break;
	}

}

