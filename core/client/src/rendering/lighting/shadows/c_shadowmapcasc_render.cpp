#include "stdafx_client.h"
#include "pragma/rendering/lighting/shadows/c_shadowmapcasc.h"
#include "pragma/console/c_cvar.h"
#include <image/prosper_render_target.hpp>

extern DLLCENGINE CEngine *c_engine;
ShadowMapCasc::TextureSet::TextureSet()
{}

static CVar cvDynamicShadows = GetClientConVar("cl_render_shadow_dynamic");

void ShadowMapCasc::DestroyTextures()
{
	for(auto &set : m_textureSets)
		set.renderTarget = nullptr;
}

void ShadowMapCasc::InitializeDepthTextures(uint32_t size)
{
	auto bDynamic = (m_bUseDualTextureSet && cvDynamicShadows->GetBool()) ? true : false;
	auto format = Anvil::Format::D32_SFLOAT;
	auto layerCount = m_layerCount;
	auto type = GetType();
	for(auto i=decltype(m_textureSets.size()){0};i<m_textureSets.size();++i)
	{
		auto rp = static_cast<pragma::CLightComponent::RenderPass>(i);
		if(rp == pragma::CLightComponent::RenderPass::Dynamic && bDynamic == false)
			continue;
		InitializeTextureSet(m_textureSets.at(i),rp);
	}
	//context.FlushInitCmd(); // prosper TODO
}

void ShadowMapCasc::ReloadDepthTextures()
{
	//m_framebuffersKeep.clear(); // prosper TODO
	//m_renderPassKeep = nullptr; // prosper TODO
	c_engine->WaitIdle(); // Wait until all active rendering commands have been completed
	for(auto &t : m_textureSets)
		t.renderTarget = nullptr;
	ShadowMap::ReloadDepthTextures();
	if(IsValid() == false)
		return;
	InitializeTextureSet(m_pendingInfo.staticTextureSet,pragma::CLightComponent::RenderPass::Static);
	auto &set = m_textureSets[umath::to_integral(pragma::CLightComponent::RenderPass::Dynamic)];
	if(set.renderTarget == nullptr)
		return;
	auto &depthTexture = set.renderTarget->GetTexture();
	auto &sampler = depthTexture->GetSampler();
	auto layerCount = GetLayerCount();
	/*for(auto i=decltype(layerCount){0};i<layerCount;++i)
		m_descSet->Update(0,i,sampler,depthTexture->GetImageView(i));
	m_renderPassKeep = context.GenerateRenderPass({
		{static_cast<vk::Format>(depthTexture->GetImage()->get_image_format()),Anvil::SampleCountFlagBits::_1_BIT,false}
	});
	m_framebuffersKeep.reserve(set.framebuffers.size());
	for(auto &framebuffer : set.framebuffers)
		m_framebuffersKeep.push_back(Vulkan::Framebuffer::Create(context,m_renderPassKeep,framebuffer->GetWidth(),framebuffer->GetHeight(),framebuffer->GetAttachments()));

	context.FlushInitCmd();*/ // prosper TODO
}

void ShadowMapCasc::FreeRenderTarget() {}

//const std::vector<std::shared_ptr<Anvil::Framebuffer>> &ShadowMapCasc::GetFramebuffers(CLightBase::RenderPass rp) const
//{
//	return m_textureSets[umath::to_integral(rp)].framebuffers;
//} // prosper TODO
const std::shared_ptr<prosper::RenderPass> &ShadowMapCasc::GetRenderPass(pragma::CLightComponent::RenderPass rp) const
{
	auto &set = m_textureSets[umath::to_integral(rp)];
	static std::shared_ptr<prosper::RenderPass> nptr = nullptr;
	return set.renderTarget ? set.renderTarget->GetRenderPass() : nptr;
}
const std::shared_ptr<prosper::Texture> &ShadowMapCasc::GetDepthTexture(pragma::CLightComponent::RenderPass rp) const
{
	auto &set = m_textureSets.at(umath::to_integral(rp));
	static std::shared_ptr<prosper::Texture> nptr = nullptr;
	return set.renderTarget ? set.renderTarget->GetTexture() : nptr;
}
const std::shared_ptr<prosper::Texture> &ShadowMapCasc::GetDepthTexture() const {return GetDepthTexture(pragma::CLightComponent::RenderPass::Dynamic);}
const std::shared_ptr<prosper::RenderTarget> &ShadowMapCasc::GetRenderTarget(pragma::CLightComponent::RenderPass rp) const
{
	return m_textureSets[umath::to_integral(rp)].renderTarget;
}
const std::shared_ptr<prosper::RenderTarget> &ShadowMapCasc::GetStaticPendingRenderTarget() const {return m_pendingInfo.staticTextureSet.renderTarget;}
const Mat4 &ShadowMapCasc::GetStaticPendingViewProjectionMatrix(uint32_t layer) const {return m_pendingInfo.prevVpMatrices[layer];}

//const std::shared_ptr<Anvil::Framebuffer> &ShadowMapCasc::GetStaticPendingFramebuffer(uint32_t layer) const {return m_pendingInfo.staticTextureSet.framebuffers.at(layer);}
//const std::vector<std::shared_ptr<Anvil::Framebuffer>> &ShadowMapCasc::GetStaticPendingFramebuffers() const {return m_pendingInfo.staticTextureSet.framebuffers;}
//const std::shared_ptr<Anvil::RenderPass> &ShadowMapCasc::GetStaticPendingRenderPass() const {return m_pendingInfo.staticTextureSet.renderPass;}
//const std::shared_ptr<prosper::Texture> &ShadowMapCasc::GetStaticPendingDepthTexture() const {return m_pendingInfo.staticTextureSet.depthTexture;}

const std::shared_ptr<prosper::Framebuffer> &ShadowMapCasc::GetFramebuffer(pragma::CLightComponent::RenderPass rp,uint32_t layer) const
{
	auto &set = m_textureSets[umath::to_integral(rp)];
	assert(layer < set.framebuffers.size());
	return set.renderTarget->GetFramebuffer(layer);
}

bool ShadowMapCasc::IsDynamicValid() const {return (GetDepthTexture(pragma::CLightComponent::RenderPass::Dynamic) != nullptr) ? true : false;} // prosper TODO
