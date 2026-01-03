// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :rendering.forward_plus;
import :console.register_commands;
import :debug;
import :engine;
import :entities.components;
import :game;
import :gui;
import :rendering.light_data_buffer_manager;
import :rendering.shaders;

static void cmd_forwardplus_tile_size(pragma::NetworkState *, const pragma::console::ConVar &, int32_t, int32_t val)
{
	if(pragma::get_cgame() == nullptr)
		return;

	pragma::ShaderForwardPLightCulling::TILE_SIZE = val;
	for(auto &c : EntityCIterator<pragma::CRasterizationRendererComponent> {*pragma::get_cgame()}) {
		auto &fp = c.GetForwardPlusInstance();
		auto &prepass = c.GetPrepass();
		pragma::get_cengine()->GetRenderContext().WaitIdle();
		fp.Initialize(pragma::get_cengine()->GetRenderContext(), c.GetWidth(), c.GetHeight(), *prepass.textureDepth);
		auto cRenderer = c.GetRendererComponent<pragma::CRendererComponent>();
		if(cRenderer)
			cRenderer->UpdateRenderSettings();
	}

	pragma::get_cengine()->ReloadShader("forwardp_light_culling");
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<int32_t>("render_forwardplus_tile_size", &cmd_forwardplus_tile_size);
};

static auto cvTileSize = pragma::console::get_client_con_var("render_forwardplus_tile_size");
std::pair<uint32_t, uint32_t> pragma::rendering::ForwardPlusInstance::CalcWorkGroupCount(uint32_t w, uint32_t h)
{
	const auto tileSize = cvTileSize->GetInt();
	const auto fCalcWorkGroups = [tileSize](uint32_t v) {
		auto numGroups = v;
		if((numGroups % tileSize) == 0)
			numGroups /= tileSize;
		else
			numGroups = (numGroups + (tileSize - (numGroups % tileSize))) / tileSize;
		return numGroups;
	};

	return {fCalcWorkGroups(w), fCalcWorkGroups(h)};
}
uint32_t pragma::rendering::ForwardPlusInstance::CalcTileCount(uint32_t w, uint32_t h)
{
	auto workGroupCount = CalcWorkGroupCount(w, h);
	return workGroupCount.first * workGroupCount.second;
}

static constexpr uint32_t get_shadow_integer_count()
{
	// Minimum number of integers required to contain 1 bit for each light with a shadow
	return pragma::math::to_integral(pragma::GameLimits::MaxAbsoluteShadowLights) / 32u + 1u;
}

pragma::rendering::ForwardPlusInstance::ForwardPlusInstance(CRasterizationRendererComponent &rasterizer) : m_rasterizer {rasterizer}
{
	m_cmdBuffer = get_cengine()->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Compute, m_cmdBufferQueueFamilyIndex);

	m_shaderLightCulling = get_cengine()->GetShader("forwardp_light_culling");
}

bool pragma::rendering::ForwardPlusInstance::Initialize(prosper::IPrContext &context, uint32_t width, uint32_t height, prosper::Texture &depthTexture)
{
	if(ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDERER.IsValid() == false)
		return false;
	auto workGroupCount = CalcWorkGroupCount(width, height);
	m_workGroupCountX = workGroupCount.first;
	m_workGroupCountY = workGroupCount.second;
	m_tileCount = CalcTileCount(width, height);

	using VisibleIndex = int32_t;
	auto numLights = math::to_integral(GameLimits::MaxAbsoluteLights);
	auto numTiles = m_tileCount * numLights;

	std::vector<VisibleIndex> defaultIndices(numTiles, -1);
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = defaultIndices.size() * sizeof(defaultIndices.front());
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	m_bufTileVisLightIndex = context.CreateBuffer(createInfo, defaultIndices.data());
	m_bufTileVisLightIndex->SetDebugName("tile_vis_light_index_buf");

	m_shadowLightBits.resize(get_shadow_integer_count(), 0);
	createInfo.size = m_shadowLightBits.size() * sizeof(m_shadowLightBits.front());
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;
	m_bufVisLightIndex = context.CreateBuffer(createInfo, m_shadowLightBits.data());
	m_bufVisLightIndex->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::ReadBit);
	m_bufVisLightIndex->SetDebugName("vis_light_index_buf");

	m_rasterizer.GetRendererDescriptorSet()->SetBindingStorageBuffer(*m_bufTileVisLightIndex, math::to_integral(ShaderGameWorldLightingPass::RendererBinding::TileVisLightIndexBuffer));

	auto &descSetCompute = *m_rasterizer.GetLightSourceDescriptorSetCompute();
	descSetCompute.SetBindingStorageBuffer(*m_bufTileVisLightIndex, math::to_integral(ShaderForwardPLightCulling::LightBinding::TileVisLightIndexBuffer));
	descSetCompute.SetBindingTexture(depthTexture, math::to_integral(ShaderForwardPLightCulling::LightBinding::DepthMap));
	descSetCompute.SetBindingStorageBuffer(*m_bufVisLightIndex, math::to_integral(ShaderForwardPLightCulling::LightBinding::VisLightIndexBuffer));
	return true;
}

void pragma::rendering::ForwardPlusInstance::Compute(prosper::IPrimaryCommandBuffer &cmdBuffer, CSceneComponent &scene, prosper::IImage &imgDepth, prosper::IDescriptorSet &descSetCam)
{
	if(m_shaderLightCulling.expired() || m_shadowLightBits.empty() == true)
		return;
	auto &shaderLightCulling = static_cast<ShaderForwardPLightCulling &>(*m_shaderLightCulling.get());
	prosper::ShaderBindState bindState {cmdBuffer};
	if(shaderLightCulling.RecordBeginCompute(bindState) == false)
		return;

	auto &bufLightSources = CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = CLightComponent::GetGlobalShadowBuffer();
	// Light source data barrier
	cmdBuffer.RecordBufferBarrier(bufLightSources, prosper::PipelineStageFlags::ComputeShaderBit | prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);
	// Shadow data barrier
	cmdBuffer.RecordBufferBarrier(bufShadowData, prosper::PipelineStageFlags::ComputeShaderBit | prosper::PipelineStageFlags::FragmentShaderBit | prosper::PipelineStageFlags::TransferBit, prosper::PipelineStageFlags::ComputeShaderBit,
	  prosper::AccessFlags::ShaderReadBit | prosper::AccessFlags::TransferWriteBit, prosper::AccessFlags::ShaderReadBit);

	// Visible light tile index buffer
	cmdBuffer.RecordBufferBarrier(*m_rasterizer.GetForwardPlusInstance().GetTileVisLightIndexBuffer(), prosper::PipelineStageFlags::FragmentShaderBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::ShaderReadBit, prosper::AccessFlags::ShaderWriteBit);

	// Visible light index buffer
	cmdBuffer.RecordBufferBarrier(*m_rasterizer.GetForwardPlusInstance().GetVisLightIndexBuffer(), prosper::PipelineStageFlags::HostBit, prosper::PipelineStageFlags::ComputeShaderBit, prosper::AccessFlags::HostReadBit, prosper::AccessFlags::ShaderWriteBit);

	auto workGroupCount = GetWorkGroupCount();
	auto sceneIndex = scene.GetSceneIndex();
	auto vpWidth = m_rasterizer.GetWidth();
	auto vpHeight = m_rasterizer.GetHeight();
	auto &instance = LightDataBufferManager::GetInstance();
	if(shaderLightCulling.RecordCompute(bindState, *m_rasterizer.GetLightSourceDescriptorSetCompute(), descSetCam, vpWidth, vpHeight, workGroupCount.first, workGroupCount.second, instance.GetLightDataBufferCount(), sceneIndex) == false)
		return;

	// Visible light index buffer
	cmdBuffer.RecordBufferBarrier(*m_rasterizer.GetForwardPlusInstance().GetVisLightIndexBuffer(), prosper::PipelineStageFlags::ComputeShaderBit, prosper::PipelineStageFlags::HostBit, prosper::AccessFlags::ShaderWriteBit, prosper::AccessFlags::HostReadBit);

	shaderLightCulling.RecordEndCompute(bindState);
	const auto szRead = m_shadowLightBits.size() * sizeof(m_shadowLightBits.front());
	m_bufVisLightIndex->Read(0ull, szRead, m_shadowLightBits.data());
}

const std::vector<uint32_t> &pragma::rendering::ForwardPlusInstance::GetShadowLightBits() const { return m_shadowLightBits; }
std::pair<uint32_t, uint32_t> pragma::rendering::ForwardPlusInstance::GetWorkGroupCount() const { return {m_workGroupCountX, m_workGroupCountY}; }
uint32_t pragma::rendering::ForwardPlusInstance::GetTileCount() const { return m_tileCount; }
prosper::IDescriptorSet *pragma::rendering::ForwardPlusInstance::GetDepthDescriptorSetGraphics() const { return m_dsgSceneDepthBuffer->GetDescriptorSet(); }
const std::shared_ptr<prosper::IBuffer> &pragma::rendering::ForwardPlusInstance::GetTileVisLightIndexBuffer() const { return m_bufTileVisLightIndex; }
const std::shared_ptr<prosper::IBuffer> &pragma::rendering::ForwardPlusInstance::GetVisLightIndexBuffer() const { return m_bufVisLightIndex; }
