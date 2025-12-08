// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :game;
import :engine;
import :entities.components;

pragma::BaseEnvLightDirectionalComponent *pragma::CGame::GetEnvironmentLightSource() const { return const_cast<pragma::BaseEnvLightDirectionalComponent *>(m_hEnvLight.get()); }
void pragma::CGame::UpdateEnvironmentLightSource()
{
	auto *oldLightSource = m_hEnvLight.get();
	pragma::ecs::EntityIterator entIt {*pragma::get_cgame()};
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
void pragma::CGame::OnEnvironmentLightSourceChanged(pragma::BaseEnvLightDirectionalComponent *oldSource, pragma::BaseEnvLightDirectionalComponent *newSource)
{
	CallCallbacks<void, pragma::CLightDirectionalComponent *, pragma::CLightDirectionalComponent *>("OnEnvironmentLightSourceChanged", static_cast<pragma::CLightDirectionalComponent *>(oldSource), static_cast<pragma::CLightDirectionalComponent *>(newSource));
}
