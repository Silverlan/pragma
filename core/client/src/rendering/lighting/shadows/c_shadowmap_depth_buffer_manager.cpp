#include "stdafx_client.h"
#include "pragma/rendering/lighting/shadows/c_shadowmap_depth_buffer_manager.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include <pragma/game/game_limits.h>
#include <image/prosper_texture.hpp>
#include <prosper_util.hpp>
#include <image/prosper_sampler.hpp>
#include <image/prosper_image_view.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <image/prosper_render_target.hpp>

extern DLLCENGINE CEngine *c_engine;

#pragma optimize("",off)
ShadowMapDepthBufferManager::ShadowMapDepthBufferManager()
{
	m_genericSet.limit = umath::to_integral(GameLimits::MaxActiveShadowMaps);
	m_genericSet.buffers.reserve(m_genericSet.limit);

	m_cubeSet.limit = umath::to_integral(GameLimits::MaxActiveShadowCubeMaps);
	m_cubeSet.buffers.reserve(m_cubeSet.limit);
}

void ShadowMapDepthBufferManager::Clear()
{
	c_engine->WaitIdle();
	ClearRenderTargets();
	m_descSetGroup = nullptr;
	m_whShadowShader = {};
}

void ShadowMapDepthBufferManager::Initialize()
{
	if(m_descSetGroup != nullptr || pragma::ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS.IsValid() == false)
		return;
	m_whShadowShader = c_engine->GetShader("shadow");
	m_descSetGroup = prosper::util::create_descriptor_set_group(c_engine->GetDevice(),pragma::ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS);

	auto *descSet = (*m_descSetGroup)->get_descriptor_set(0u);
	auto *descSetLayout = (*m_descSetGroup)->get_descriptor_set_layout(0u);
	auto &info = *descSetLayout->get_create_info();
	
	// Shadow map descriptor bindings need to be bound to dummy images.
	// For normal shadow maps this is already taken care of, but cubemaps are a special case
	// that needs to be dealt with
	auto arraySize = pragma::ShaderTextured3DBase::DESCRIPTOR_SET_SHADOWS.bindings.at(umath::to_integral(pragma::ShaderTextured3DBase::ShadowBinding::ShadowCubeMaps)).descriptorArraySize;
	auto &dummyTex = c_engine->GetDummyCubemapTexture();
	std::vector<Anvil::DescriptorSet::CombinedImageSamplerBindingElement> bindingElements(arraySize,Anvil::DescriptorSet::CombinedImageSamplerBindingElement{
		Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL,
		&dummyTex->GetImageView()->GetAnvilImageView(),&dummyTex->GetSampler()->GetAnvilSampler()
	});
	descSet->set_binding_array_items(umath::to_integral(pragma::ShaderTextured3DBase::ShadowBinding::ShadowCubeMaps),{0u,bindingElements.size()},bindingElements.data());
}

void ShadowMapDepthBufferManager::ClearRenderTargets()
{
	m_genericSet.buffers = {};
	m_genericSet.freeBuffers = {};

	m_cubeSet.buffers = {};
	m_cubeSet.freeBuffers = {};
}

Anvil::DescriptorSet *ShadowMapDepthBufferManager::GetDescriptorSet() {return (*m_descSetGroup)->get_descriptor_set(0u);}

std::weak_ptr<ShadowMapDepthBufferManager::RenderTarget> ShadowMapDepthBufferManager::RequestRenderTarget(Type type,uint32_t size)
{
	if(m_whShadowShader.expired())
		return {};
	auto &set = (type == Type::Cube) ? m_cubeSet : m_genericSet;
	auto layerCount = (type == Type::Cube) ? 6u : 1u;
	if(set.freeBuffers.empty() == false)
	{
		auto idx = set.freeBuffers.front();
		set.freeBuffers.pop();
		return set.buffers.at(idx);
	}
	if(set.buffers.size() >= set.limit)
		return {};

	auto &dev = c_engine->GetDevice();
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = size;
	createInfo.height = size;
	createInfo.format = pragma::ShaderShadow::RENDER_PASS_DEPTH_FORMAT;
	createInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT;
	createInfo.layers = layerCount;
	createInfo.postCreateLayout = Anvil::ImageLayout::SHADER_READ_ONLY_OPTIMAL;
	if(type == Type::Cube)
		createInfo.flags = prosper::util::ImageCreateInfo::Flags::Cubemap;
	auto img = prosper::util::create_image(dev,createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	//samplerCreateInfo.compareEnable = true; // When enabled, causes strange behavior on Nvidia cards when doing texture lookups
	//samplerCreateInfo.compareOp = Anvil::CompareOp::LESS_OR_EQUAL;
	samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.borderColor = Anvil::BorderColor::FLOAT_OPAQUE_WHITE;
	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer;
	auto depthTexture = prosper::util::create_texture(dev,texCreateInfo,img,&imgViewCreateInfo,&samplerCreateInfo);

	auto rt = std::make_shared<RenderTarget>();
	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	rtCreateInfo.useLayerFramebuffers = true;
	rt->renderTarget = prosper::util::create_render_target(dev,{depthTexture},static_cast<prosper::ShaderGraphics*>(m_whShadowShader.get())->GetRenderPass(),rtCreateInfo);
	rt->renderTarget->SetDebugName("shadowmap_rt");
	set.buffers.push_back(rt);
	rt->index = set.buffers.size() -1;
	prosper::util::set_descriptor_set_binding_array_texture(
		*(*m_descSetGroup)->get_descriptor_set(0u),*depthTexture,
		umath::to_integral((type != Type::Cube) ? pragma::ShaderSceneLit::ShadowBinding::ShadowMaps : pragma::ShaderSceneLit::ShadowBinding::ShadowCubeMaps),
		rt->index
	);
	return rt;
}

void ShadowMapDepthBufferManager::FreeRenderTarget(const RenderTarget &rt)
{
	auto &depthTex = rt.renderTarget->GetTexture();
	auto bCube = (depthTex->GetImage()->GetCreateFlags() &Anvil::ImageCreateFlagBits::CUBE_COMPATIBLE_BIT) != 0;
	auto &set = (bCube == true) ? m_cubeSet : m_genericSet;
	auto it = std::find_if(set.buffers.begin(),set.buffers.end(),[&rt](const std::shared_ptr<RenderTarget> &ptRtOther) {
		return (rt.renderTarget.get() == ptRtOther->renderTarget.get()) ? true : false;
	});
	if(it == set.buffers.end())
		return;
	set.freeBuffers.push(it -set.buffers.begin());
}
#pragma optimize("",on)
