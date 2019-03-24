#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap.h"
#include "pragma/rendering/uniformbinding.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include <pragma/entities/entity_iterator.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCENGINE CEngine *c_engine;

pragma::CLightDirectionalComponent *CGame::GetEnvironmentLightSource() const {return m_hEnvLight.get();}
void CGame::UpdateEnvironmentLightSource()
{
	auto *oldLightSource = m_hEnvLight.get();
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightDirectionalComponent>>();
	for(auto *ent : entIt)
	{
		auto pToggleComponent = ent->GetComponent<pragma::CToggleComponent>();
		if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false)
			continue;
		auto pLightDirComponent = ent->GetComponent<pragma::CLightDirectionalComponent>();
		pLightDirComponent->ReloadShadowCommandBuffers();
		if(pLightDirComponent.get() != oldLightSource)
		{
			m_hEnvLight = pLightDirComponent->GetHandle<pragma::CLightDirectionalComponent>();
			OnEnvironmentLightSourceChanged(oldLightSource,pLightDirComponent.get());
		}
		return;
	}
	if(oldLightSource == nullptr)
		return;
	m_hEnvLight = {};
	OnEnvironmentLightSourceChanged(oldLightSource,nullptr);
}
void CGame::OnEnvironmentLightSourceChanged(pragma::CLightDirectionalComponent *oldSource,pragma::CLightDirectionalComponent *newSource)
{
	CallCallbacks<void,pragma::CLightDirectionalComponent*,pragma::CLightDirectionalComponent*>("OnEnvironmentLightSourceChanged",oldSource,newSource);
}
