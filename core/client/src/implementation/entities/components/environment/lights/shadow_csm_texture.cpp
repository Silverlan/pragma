// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include "pragma/console/c_cvar.h"
#include "pragma/c_engine.h"
#include <image/prosper_render_target.hpp>
#include <prosper_render_pass.hpp>
#include <prosper_framebuffer.hpp>

module pragma.client.entities.components.lights.shadow_csm;

using namespace pragma;

extern CEngine *c_engine;
extern ClientState *client;
extern CGame *c_game;

CShadowCSMComponent::TextureSet::TextureSet() {}

static CVar cvDynamicShadows = GetClientConVar("cl_render_shadow_dynamic");

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
		auto rp = static_cast<pragma::CLightComponent::ShadowMapType>(i);
		if(rp == pragma::CLightComponent::ShadowMapType::Dynamic && bDynamic == false)
			continue;
		InitializeTextureSet(m_textureSets.at(i), rp);
	}
}

static CVar cvShadowmapSize = GetClientConVar("cl_render_shadow_resolution");
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
	InitializeTextureSet(m_pendingInfo.staticTextureSet, pragma::CLightComponent::ShadowMapType::Static);
}

void CShadowCSMComponent::FreeRenderTarget() {}

prosper::IRenderPass *CShadowCSMComponent::GetRenderPass(pragma::CLightComponent::ShadowMapType smType) const
{
	auto &set = m_textureSets.at(umath::to_integral(smType));
	return set.renderTarget ? &set.renderTarget->GetRenderPass() : nullptr;
}
prosper::Texture *CShadowCSMComponent::GetDepthTexture(pragma::CLightComponent::ShadowMapType rp) const
{
	auto &set = m_textureSets.at(umath::to_integral(rp));
	return set.renderTarget ? &set.renderTarget->GetTexture() : nullptr;
}
prosper::Texture *CShadowCSMComponent::GetDepthTexture() const { return GetDepthTexture(pragma::CLightComponent::ShadowMapType::Dynamic); }
const std::shared_ptr<prosper::RenderTarget> &CShadowCSMComponent::GetRenderTarget(pragma::CLightComponent::ShadowMapType smType) const { return m_textureSets.at(umath::to_integral(smType)).renderTarget; }
const std::shared_ptr<prosper::RenderTarget> &CShadowCSMComponent::GetStaticPendingRenderTarget() const { return m_pendingInfo.staticTextureSet.renderTarget; }
const Mat4 &CShadowCSMComponent::GetStaticPendingViewProjectionMatrix(uint32_t layer) const { return m_pendingInfo.prevVpMatrices[layer]; }

std::shared_ptr<prosper::IFramebuffer> CShadowCSMComponent::GetFramebuffer(pragma::CLightComponent::ShadowMapType smType, uint32_t layer) const
{
	auto &set = m_textureSets.at(umath::to_integral(smType));
	auto *fb = set.renderTarget->GetFramebuffer(layer);
	assert(fb != nullptr);
	return fb ? fb->shared_from_this() : nullptr;
}

bool CShadowCSMComponent::IsDynamicValid() const { return (GetDepthTexture(pragma::CLightComponent::ShadowMapType::Dynamic) != nullptr) ? true : false; }
