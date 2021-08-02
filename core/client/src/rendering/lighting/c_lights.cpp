/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/components/c_toggle_component.hpp"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT CEngine *c_engine;

pragma::CLightDirectionalComponent *CGame::GetEnvironmentLightSource() const {return const_cast<pragma::CLightDirectionalComponent*>(m_hEnvLight.get());}
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
	m_hEnvLight = decltype(m_hEnvLight){};
	OnEnvironmentLightSourceChanged(oldLightSource,nullptr);
}
void CGame::OnEnvironmentLightSourceChanged(pragma::CLightDirectionalComponent *oldSource,pragma::CLightDirectionalComponent *newSource)
{
	CallCallbacks<void,pragma::CLightDirectionalComponent*,pragma::CLightDirectionalComponent*>("OnEnvironmentLightSourceChanged",oldSource,newSource);
}
