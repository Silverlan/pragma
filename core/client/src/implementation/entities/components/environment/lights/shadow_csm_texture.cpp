// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :entities.components.lights.shadow_csm;
import :client_state;
import :engine;

using namespace pragma;

CShadowCSMComponent::TextureSet::TextureSet() {}

static auto cvDynamicShadows = console::get_client_con_var("cl_render_shadow_dynamic");

void CShadowCSMComponent::DestroyTextures()
{
	for(auto &set : m_textureSets)
		set.renderTarget = nullptr;
}

void CShadowCSMComponent::InitializeDepthTextures(uint32_t size)
{
	auto bDynamic = true; //(m_bUseDualTextureSet && cvDynamicShadows->GetBool()) ? true : false;
	auto format = prosper::Format::D32_SFloat;
	auto layerCount = m_layerCount;
	for(auto i = decltype(m_textureSets.size()) {0}; i < m_textureSets.size(); ++i) {
		auto rp = static_cast<rendering::ShadowMapType>(i);
		if(rp == rendering::ShadowMapType::Dynamic && bDynamic == false)
			continue;
		InitializeTextureSet(m_textureSets.at(i), rp);
	}
}

static auto cvShadowmapSize = console::get_client_con_var("cl_render_shadow_resolution");
void CShadowCSMComponent::ReloadDepthTextures()
{
	for(auto &t : m_textureSets)
		t.renderTarget = nullptr;
	/*
	volatile ScopeGuard sg {[this]() {
		if(m_onTexturesReloaded == nullptr)
			return;
		m_onTexturesReloaded();
	}};
	if(cvShadowQuality->GetInt() <= 0)
		return;
	*/
	auto size = cvShadowmapSize->GetInt();
	if(size > 0)
		InitializeDepthTextures(size);
	InitializeTextureSet(m_pendingInfo.staticTextureSet, rendering::ShadowMapType::Static);
}

void CShadowCSMComponent::FreeRenderTarget() {}

prosper::IRenderPass *CShadowCSMComponent::GetRenderPass(rendering::ShadowMapType smType) const
{
	auto &set = m_textureSets.at(math::to_integral(smType));
	return set.renderTarget ? &set.renderTarget->GetRenderPass() : nullptr;
}
prosper::Texture *CShadowCSMComponent::GetDepthTexture(rendering::ShadowMapType rp) const
{
	auto &set = m_textureSets.at(math::to_integral(rp));
	return set.renderTarget ? &set.renderTarget->GetTexture() : nullptr;
}
prosper::Texture *CShadowCSMComponent::GetDepthTexture() const { return GetDepthTexture(rendering::ShadowMapType::Dynamic); }
const std::shared_ptr<prosper::RenderTarget> &CShadowCSMComponent::GetRenderTarget(rendering::ShadowMapType smType) const { return m_textureSets.at(math::to_integral(smType)).renderTarget; }
const std::shared_ptr<prosper::RenderTarget> &CShadowCSMComponent::GetStaticPendingRenderTarget() const { return m_pendingInfo.staticTextureSet.renderTarget; }
const Mat4 &CShadowCSMComponent::GetStaticPendingViewProjectionMatrix(uint32_t layer) const { return m_pendingInfo.prevVpMatrices[layer]; }

std::shared_ptr<prosper::IFramebuffer> CShadowCSMComponent::GetFramebuffer(rendering::ShadowMapType smType, uint32_t layer) const
{
	auto &set = m_textureSets.at(math::to_integral(smType));
	auto *fb = set.renderTarget->GetFramebuffer(layer);
	assert(fb != nullptr);
	return fb ? fb->shared_from_this() : nullptr;
}

bool CShadowCSMComponent::IsDynamicValid() const { return (GetDepthTexture(rendering::ShadowMapType::Dynamic) != nullptr) ? true : false; }
