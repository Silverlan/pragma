#include "stdafx_client.h"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/console/c_cvar.h"
#include <image/prosper_render_target.hpp>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

#pragma optimize("",off)
CShadowCSMComponent::TextureSet::TextureSet()
{}

static CVar cvDynamicShadows = GetClientConVar("cl_render_shadow_dynamic");

void CShadowCSMComponent::DestroyTextures()
{
	for(auto &set : m_textureSets)
		set.renderTarget = nullptr;
}

void CShadowCSMComponent::InitializeDepthTextures(uint32_t size)
{
	auto bDynamic = true;//(m_bUseDualTextureSet && cvDynamicShadows->GetBool()) ? true : false;
	auto format = Anvil::Format::D32_SFLOAT;
	auto layerCount = m_layerCount;
	for(auto i=decltype(m_textureSets.size()){0};i<m_textureSets.size();++i)
	{
		auto rp = static_cast<pragma::CLightComponent::ShadowMapType>(i);
		if(rp == pragma::CLightComponent::ShadowMapType::Dynamic && bDynamic == false)
			continue;
		InitializeTextureSet(m_textureSets.at(i),rp);
	}
}

void CShadowCSMComponent::ReloadDepthTextures()
{
	c_engine->WaitIdle(); // Wait until all active rendering commands have been completed
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
	auto size = cvShadowmapSize->GetInt();
	if(size <= 0)
		return;
	InitializeDepthTextures(size);
	*/
	InitializeTextureSet(m_pendingInfo.staticTextureSet,pragma::CLightComponent::ShadowMapType::Static);
}

void CShadowCSMComponent::FreeRenderTarget() {}

const std::shared_ptr<prosper::RenderPass> &CShadowCSMComponent::GetRenderPass(pragma::CLightComponent::ShadowMapType smType) const
{
	auto &set = m_textureSets.at(umath::to_integral(smType));
	static std::shared_ptr<prosper::RenderPass> nptr = nullptr;
	return set.renderTarget ? set.renderTarget->GetRenderPass() : nptr;
}
const std::shared_ptr<prosper::Texture> &CShadowCSMComponent::GetDepthTexture(pragma::CLightComponent::ShadowMapType rp) const
{
	auto &set = m_textureSets.at(umath::to_integral(rp));
	static std::shared_ptr<prosper::Texture> nptr = nullptr;
	return set.renderTarget ? set.renderTarget->GetTexture() : nptr;
}
const std::shared_ptr<prosper::Texture> &CShadowCSMComponent::GetDepthTexture() const {return GetDepthTexture(pragma::CLightComponent::ShadowMapType::Dynamic);}
const std::shared_ptr<prosper::RenderTarget> &CShadowCSMComponent::GetRenderTarget(pragma::CLightComponent::ShadowMapType smType) const
{
	return m_textureSets.at(umath::to_integral(smType)).renderTarget;
}
const std::shared_ptr<prosper::RenderTarget> &CShadowCSMComponent::GetStaticPendingRenderTarget() const {return m_pendingInfo.staticTextureSet.renderTarget;}
const Mat4 &CShadowCSMComponent::GetStaticPendingViewProjectionMatrix(uint32_t layer) const {return m_pendingInfo.prevVpMatrices[layer];}

//const std::shared_ptr<Anvil::Framebuffer> &CShadowCSMComponent::GetStaticPendingFramebuffer(uint32_t layer) const {return m_pendingInfo.staticTextureSet.framebuffers.at(layer);}
//const std::vector<std::shared_ptr<Anvil::Framebuffer>> &CShadowCSMComponent::GetStaticPendingFramebuffers() const {return m_pendingInfo.staticTextureSet.framebuffers;}
//const std::shared_ptr<Anvil::RenderPass> &CShadowCSMComponent::GetStaticPendingRenderPass() const {return m_pendingInfo.staticTextureSet.renderPass;}
//const std::shared_ptr<prosper::Texture> &CShadowCSMComponent::GetStaticPendingDepthTexture() const {return m_pendingInfo.staticTextureSet.depthTexture;}

const std::shared_ptr<prosper::Framebuffer> &CShadowCSMComponent::GetFramebuffer(pragma::CLightComponent::ShadowMapType smType,uint32_t layer) const
{
	auto &set = m_textureSets.at(umath::to_integral(smType));
	assert(layer < set.framebuffers.size());
	return set.renderTarget->GetFramebuffer(layer);
}

bool CShadowCSMComponent::IsDynamicValid() const {return (GetDepthTexture(pragma::CLightComponent::ShadowMapType::Dynamic) != nullptr) ? true : false;} // prosper TODO
#pragma optimize("",on)
