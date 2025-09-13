// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

import pragma.client.entities.components;

extern CGame *c_game;
extern CEngine *c_engine;

pragma::BaseEnvLightDirectionalComponent *CGame::GetEnvironmentLightSource() const { return const_cast<pragma::BaseEnvLightDirectionalComponent *>(m_hEnvLight.get()); }
void CGame::UpdateEnvironmentLightSource()
{
	auto *oldLightSource = m_hEnvLight.get();
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightDirectionalComponent>>();
	for(auto *ent : entIt) {
		auto pToggleComponent = ent->GetComponent<pragma::CToggleComponent>();
		if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false)
			continue;
		auto pLightDirComponent = ent->GetComponent<pragma::CLightDirectionalComponent>();
		pLightDirComponent->ReloadShadowCommandBuffers();
		if(pLightDirComponent.get() != oldLightSource) {
			m_hEnvLight = pLightDirComponent->GetHandle<pragma::BaseEnvLightDirectionalComponent>();
			OnEnvironmentLightSourceChanged(oldLightSource, pLightDirComponent.get());
		}
		return;
	}
	if(oldLightSource == nullptr)
		return;
	m_hEnvLight = decltype(m_hEnvLight) {};
	OnEnvironmentLightSourceChanged(oldLightSource, nullptr);
}
void CGame::OnEnvironmentLightSourceChanged(pragma::BaseEnvLightDirectionalComponent *oldSource, pragma::BaseEnvLightDirectionalComponent *newSource)
{
	CallCallbacks<void, pragma::CLightDirectionalComponent *, pragma::CLightDirectionalComponent *>("OnEnvironmentLightSourceChanged", static_cast<pragma::CLightDirectionalComponent*>(oldSource), static_cast<pragma::CLightDirectionalComponent*>(newSource));
}
