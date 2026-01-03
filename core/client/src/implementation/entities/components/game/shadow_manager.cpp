// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;
import :client_state;

import :entities.components.game_shadow_manager;

using namespace pragma;

static CShadowManagerComponent *g_shadowManager = nullptr;
CShadowManagerComponent *CShadowManagerComponent::GetShadowManager() { return g_shadowManager; }

void CShadowManagerComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CShadowManagerComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	m_genericSet.limit = math::to_integral(GameLimits::MaxActiveShadowMaps);
	m_genericSet.buffers.reserve(m_genericSet.limit);

	m_cubeSet.limit = math::to_integral(GameLimits::MaxActiveShadowCubeMaps);
	m_cubeSet.buffers.reserve(m_cubeSet.limit);

	if(m_descSetGroup != nullptr || ShaderPBR::DESCRIPTOR_SET_SHADOWS.IsValid() == false)
		return;
	g_shadowManager = this;
	m_whShadowShader = get_cengine()->GetShader("shadow");
	m_descSetGroup = get_cengine()->GetRenderContext().CreateDescriptorSetGroup(ShaderPBR::DESCRIPTOR_SET_SHADOWS);

	auto *descSet = m_descSetGroup->GetDescriptorSet();

	// Shadow map descriptor bindings need to be bound to dummy images.
	// For normal shadow maps this is already taken care of, but cubemaps are a special case
	// that needs to be dealt with
	auto arraySizeShadows = ShaderPBR::DESCRIPTOR_SET_SHADOWS.bindings.at(math::to_integral(ShaderPBR::ShadowBinding::ShadowMaps)).descriptorArraySize;
	auto arraySizeCubeShadows = ShaderPBR::DESCRIPTOR_SET_SHADOWS.bindings.at(math::to_integral(ShaderPBR::ShadowBinding::ShadowCubeMaps)).descriptorArraySize;

	auto &dummyTex = get_cengine()->GetRenderContext().GetDummyTexture();
	for(auto i = decltype(arraySizeShadows) {0u}; i < arraySizeShadows; ++i)
		descSet->SetBindingArrayTexture(*dummyTex, math::to_integral(ShaderSceneLit::ShadowBinding::ShadowMaps), i);

	auto &dummyCubeTex = get_cengine()->GetRenderContext().GetDummyCubemapTexture();
	for(auto i = decltype(arraySizeCubeShadows) {0u}; i < arraySizeCubeShadows; ++i)
		descSet->SetBindingArrayTexture(*dummyCubeTex, math::to_integral(ShaderSceneLit::ShadowBinding::ShadowCubeMaps), i);
	descSet->Update();
}

void CShadowManagerComponent::OnRemove()
{
	BaseEntityComponent::OnRemove();

	get_cengine()->GetRenderContext().WaitIdle();
	ClearRenderTargets();
	m_descSetGroup = nullptr;
	m_whShadowShader = {};
	g_shadowManager = nullptr;
}

void CShadowManagerComponent::OnEntitySpawn() { BaseEntityComponent::OnEntitySpawn(); }

void CShadowManagerComponent::ClearRenderTargets()
{
	m_genericSet.buffers = {};
	m_cubeSet.buffers = {};
}

ShadowRenderer &CShadowManagerComponent::GetRenderer() { return m_renderer; }

prosper::IDescriptorSet *CShadowManagerComponent::GetDescriptorSet() { return m_descSetGroup->GetDescriptorSet(); }

CShadowManagerComponent::RtHandle CShadowManagerComponent::RequestRenderTarget(Type type, uint32_t size, Priority priority)
{
	if(m_whShadowShader.expired())
		return {};
	auto &set = (type == Type::Cube) ? m_cubeSet : m_genericSet;
	auto itBuffer = std::find_if(set.buffers.begin(), set.buffers.end(), [priority](const BufferSet::BufferData &data) { return priority > data.lastPriority; });
	if(itBuffer != set.buffers.end()) {
		// Found an existing target with a lower priority; We can claim this one
		auto &data = *itBuffer;
		data.renderTargetHandle = nullptr; // Invalidate previous handle (Reclaim ownership)

		// Create new handle
		data.renderTargetHandle = pragma::util::make_shared<std::weak_ptr<RenderTarget>>(data.renderTarget);
		data.lastPriority = priority;
		return data.renderTargetHandle;
	}
	if(set.buffers.size() >= set.limit)
		return {};

	spdlog::info("Initializing new shadow map...");
	auto layerCount = (type == Type::Cube) ? 6u : 1u;
	prosper::util::ImageCreateInfo createInfo {};
	createInfo.width = size;
	createInfo.height = size;
	createInfo.format = ShaderShadow::RENDER_PASS_DEPTH_FORMAT;
	createInfo.usage = prosper::ImageUsageFlags::SampledBit | prosper::ImageUsageFlags::DepthStencilAttachmentBit | prosper::ImageUsageFlags::TransferDstBit;
	createInfo.layers = layerCount;
	createInfo.postCreateLayout = prosper::ImageLayout::ShaderReadOnlyOptimal;
	if(type == Type::Cube)
		createInfo.flags = prosper::util::ImageCreateInfo::Flags::Cubemap;
	auto img = get_cengine()->GetRenderContext().CreateImage(createInfo);
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	//samplerCreateInfo.compareEnable = true; // When enabled, causes strange behavior on Nvidia cards when doing texture lookups
	//samplerCreateInfo.compareOp = Anvil::CompareOp::LESS_OR_EQUAL;
	samplerCreateInfo.addressModeU = prosper::SamplerAddressMode::ClampToBorder;
	samplerCreateInfo.addressModeV = prosper::SamplerAddressMode::ClampToBorder;
	samplerCreateInfo.addressModeW = prosper::SamplerAddressMode::ClampToBorder;
	samplerCreateInfo.borderColor = prosper::BorderColor::FloatOpaqueWhite;

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::CreateImageViewForEachLayer;
	auto depthTexture = get_cengine()->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);

	auto rt = pragma::util::make_shared<RenderTarget>();
	prosper::util::RenderTargetCreateInfo rtCreateInfo {};
	rtCreateInfo.useLayerFramebuffers = true;
	rt->renderTarget = get_cengine()->GetRenderContext().CreateRenderTarget({depthTexture}, static_cast<prosper::ShaderGraphics *>(m_whShadowShader.get())->GetRenderPass(), rtCreateInfo);
	rt->renderTarget->SetDebugName("shadowmap_rt");
	set.buffers.push_back({});
	auto &data = set.buffers.back();
	data.lastPriority = priority;
	data.renderTarget = rt;
	data.renderTargetHandle = pragma::util::make_shared<std::weak_ptr<RenderTarget>>(data.renderTarget);

	rt->index = set.buffers.size() - 1;
	m_descSetGroup->GetDescriptorSet()->SetBindingArrayTexture(*depthTexture, math::to_integral((type != Type::Cube) ? ShaderSceneLit::ShadowBinding::ShadowMaps : ShaderSceneLit::ShadowBinding::ShadowCubeMaps), rt->index);
	m_descSetGroup->GetDescriptorSet()->Update();
	return data.renderTargetHandle;
}

void CShadowManagerComponent::FreeRenderTarget(const RenderTarget &rt)
{
	auto &depthTex = rt.renderTarget->GetTexture();
	auto bCube = depthTex.GetImage().IsCubemap();
	auto &set = (bCube == true) ? m_cubeSet : m_genericSet;
	auto &data = set.buffers.at(rt.index);
	data.lastPriority = -1;
	data.renderTargetHandle = nullptr;
}

void CShadowManagerComponent::UpdatePriority(const RenderTarget &rt, Priority priority)
{
	auto &depthTex = rt.renderTarget->GetTexture();
	auto bCube = depthTex.GetImage().IsCubemap();
	auto &set = (bCube == true) ? m_cubeSet : m_genericSet;
	auto &data = set.buffers.at(rt.index);
	data.lastPriority = priority;
}

////////

void CShadowManager::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CShadowManagerComponent>();
}
