// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.debug_shadow_map;

import :client_state;
import :engine;
import :entities.components;
import :game;

pragma::gui::types::WIDebugShadowMap::WIDebugShadowMap() : WIBase() {}

void pragma::gui::types::WIDebugShadowMap::SetContrastFactor(float contrastFactor)
{
	m_contrastFactor = contrastFactor;
	for(auto &hEl : m_shadowMapImages) {
		auto *el = dynamic_cast<WIDebugDepthTexture *>(hEl.get());
		if(el == nullptr)
			continue;
		el->SetContrastFactor(contrastFactor);
	}
}
void pragma::gui::types::WIDebugShadowMap::SetShadowMapType(rendering::ShadowMapType type) { m_shadowMapType = type; }
float pragma::gui::types::WIDebugShadowMap::GetContrastFactor() const { return m_contrastFactor; }

void pragma::gui::types::WIDebugShadowMap::SetShadowMapSize(uint32_t w, uint32_t h) { m_shadowMapSize = {w, h}; }

void pragma::gui::types::WIDebugShadowMap::SetLightSource(CLightComponent &lightSource) { m_lightHandle = lightSource.GetHandle<CLightComponent>(); }

void pragma::gui::types::WIDebugShadowMap::DoUpdate()
{
	WIBase::DoUpdate();
	for(auto &hEl : m_shadowMapImages) {
		if(hEl.IsValid() == false)
			continue;
		hEl->Remove();
	}
	if(get_cgame() == nullptr || m_lightHandle.expired())
		return;
	auto &lightSource = *m_lightHandle;
	auto type = LightType::Undefined;
	auto *pLight = lightSource.GetLight(type);
	if(pLight == nullptr)
		return;
	prosper::Texture *depthTexture = nullptr;
	auto hShadow = lightSource.GetShadowMap<CShadowComponent>(m_shadowMapType);
	auto hShadowCsm = lightSource.GetEntity().GetComponent<CShadowCSMComponent>();
	if(hShadow.valid())
		depthTexture = hShadow->GetDepthTexture();
	else if(hShadowCsm.valid())
		depthTexture = hShadowCsm->GetDepthTexture(m_shadowMapType);

	if(depthTexture == nullptr)
		return;
	auto &depthImage = depthTexture->GetImage();
	auto numLayers = depthImage.GetLayerCount();
	auto wLayer = m_shadowMapSize.x;
	auto hLayer = m_shadowMapSize.y;
	prosper::util::BarrierImageLayout barrierImageLayout {prosper::PipelineStageFlags::FragmentShaderBit, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::AccessFlags::ShaderReadBit};
	auto pRadiusComponent = lightSource.GetEntity().GetComponent<CRadiusComponent>();
	auto &wgui = WGUI::GetInstance();
	switch(type) {
	case LightType::Point:
		{
			for(auto i = decltype(numLayers) {0}; i < numLayers; ++i) {
				auto *dt = wgui.Create<WIDebugDepthTexture>(this);
				dt->SetTexture(*depthTexture, barrierImageLayout, barrierImageLayout, i);
				dt->SetSize(wLayer, hLayer);
				if(i == static_cast<uint32_t>(rendering::CubeMapSide::Left))
					dt->SetPos(0, hLayer); // Left
				else if(i == static_cast<uint32_t>(rendering::CubeMapSide::Right))
					dt->SetPos(wLayer * 2, hLayer); // Right
				else if(i == static_cast<uint32_t>(rendering::CubeMapSide::Top))
					dt->SetPos(wLayer, 0); // Up
				else if(i == static_cast<uint32_t>(rendering::CubeMapSide::Bottom))
					dt->SetPos(wLayer, hLayer * 2); // Down
				else if(i == static_cast<uint32_t>(rendering::CubeMapSide::Front))
					dt->SetPos(wLayer, hLayer); // Front
				else
					dt->SetPos(wLayer * 3, hLayer); // Back
				dt->Setup(1.f, pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
				dt->SetName("dbg_shadowmap" + std::to_string(i));
				dt->SetContrastFactor(GetContrastFactor());
				m_shadowMapImages.push_back(dt->GetHandle());
			}
			break;
		}
	case LightType::Spot:
		{
			auto *dt = wgui.Create<WIDebugDepthTexture>(this);
			dt->SetTexture(*depthTexture, barrierImageLayout, barrierImageLayout);
			dt->SetSize(wLayer, hLayer);
			dt->Setup(1.f, pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
			dt->SetName("dbg_shadowmap" + std::to_string(0));
			dt->SetContrastFactor(GetContrastFactor());
			m_shadowMapImages.push_back(dt->GetHandle());
			break;
		}
	case LightType::Directional:
		{
			wLayer *= 0.5f;
			hLayer *= 0.5f;
			for(auto i = decltype(numLayers) {0}; i < numLayers; ++i) {
				auto *dt = wgui.Create<WIDebugDepthTexture>(this);
				dt->SetTexture(*depthTexture, barrierImageLayout, barrierImageLayout, i);
				dt->SetSize(wLayer, hLayer);
				dt->SetPos(i * wLayer, 0);
				dt->Setup(1.f, pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
				dt->SetName("dbg_shadowmap" + std::to_string(i));
				dt->SetContrastFactor(GetContrastFactor());
				m_shadowMapImages.push_back(dt->GetHandle());
			}
			if(hShadowCsm.valid()) {
				auto &staticDepthTex = hShadowCsm->GetStaticPendingRenderTarget()->GetTexture();
				auto &staticDepthImg = staticDepthTex.GetImage();
				for(auto i = decltype(numLayers) {0}; i < numLayers; ++i) {
					auto *dt = wgui.Create<WIDebugDepthTexture>(this);
					dt->SetTexture(staticDepthTex, barrierImageLayout, barrierImageLayout, i);
					dt->SetSize(wLayer, hLayer);
					dt->SetPos(i * wLayer, hLayer);
					dt->Setup(1.f, pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f);
					dt->SetName("dbg_shadowmap_static" + std::to_string(i));
					dt->SetContrastFactor(GetContrastFactor());
					m_shadowMapImages.push_back(dt->GetHandle());
				}
			}
			hLayer *= 2.0;
			break;
		}
	default:
		break;
	}
	SizeToContents();
	auto w = GetWidth();
	auto h = GetHeight();
	for(auto &hEl : m_shadowMapImages) {
		if(hEl.IsValid() == false)
			continue;
		hEl->SetAnchor(hEl->GetLeft() / static_cast<float>(w), hEl->GetTop() / static_cast<float>(h), hEl->GetRight() / static_cast<float>(w), hEl->GetBottom() / static_cast<float>(h));
	}
}
