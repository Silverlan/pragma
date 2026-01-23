// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :game;
import :engine;
import :entities.components;

pragma::BaseEnvLightDirectionalComponent *pragma::CGame::GetEnvironmentLightSource() const { return const_cast<BaseEnvLightDirectionalComponent *>(m_hEnvLight.get()); }
void pragma::CGame::UpdateEnvironmentLightSource()
{
	auto *oldLightSource = m_hEnvLight.get();
	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CLightDirectionalComponent>>();
	for(auto *ent : entIt) {
		auto pToggleComponent = ent->GetComponent<CToggleComponent>();
		if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false)
			continue;
		auto pLightDirComponent = ent->GetComponent<CLightDirectionalComponent>();
		pLightDirComponent->ReloadShadowCommandBuffers();
		if(pLightDirComponent.get() != oldLightSource) {
			m_hEnvLight = pLightDirComponent->GetHandle<BaseEnvLightDirectionalComponent>();
			OnEnvironmentLightSourceChanged(oldLightSource, pLightDirComponent.get());
		}
		return;
	}
	if(oldLightSource == nullptr)
		return;
	m_hEnvLight = decltype(m_hEnvLight) {};
	OnEnvironmentLightSourceChanged(oldLightSource, nullptr);
}
void pragma::CGame::OnEnvironmentLightSourceChanged(BaseEnvLightDirectionalComponent *oldSource, BaseEnvLightDirectionalComponent *newSource)
{
	CallCallbacks<void, CLightDirectionalComponent *, CLightDirectionalComponent *>("OnEnvironmentLightSourceChanged", static_cast<CLightDirectionalComponent *>(oldSource), static_cast<CLightDirectionalComponent *>(newSource));
}
