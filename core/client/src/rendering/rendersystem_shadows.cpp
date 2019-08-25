#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
void RenderSystem::RenderShadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,std::vector<pragma::CLightComponent*> &lights)
{
	if(lights.empty())
		return;

	// Directional light source is always first in array, but should be rendered last, so we iterate backwards
	// Otherwise the shadowmaps for all light sources coming after are cleared; TODO: Find out why (Something to do with secondary command buffer, see CLightDirectional::GetShadowCommandBuffer / Shadow Shader)
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CShadowManagerComponent>>();
	auto it = entIt.begin();
	if(it == entIt.end())
		return;
	auto shadowManagerC = it->GetComponent<pragma::CShadowManagerComponent>();
	for(auto *pLight : lights)
		shadowManagerC->GetRenderer().RenderShadows(drawCmd,*pLight);
}
#pragma optimize("",on)
