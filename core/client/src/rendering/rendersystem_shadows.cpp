#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/rendering/rendersystem.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;


void RenderSystem::RenderShadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::rendering::RasterizationRenderer &renderer,std::vector<pragma::CLightComponent*> &lights)
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
			shadowManagerC->GetRenderer().RenderShadows(drawCmd,*pLight);
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
		shadowManagerC->GetRenderer().RenderShadows(drawCmd,*ent->GetComponent<pragma::CLightComponent>());
		break;
	}

}

