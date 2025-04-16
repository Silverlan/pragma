/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/rendering/c_forwardplus.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_indexing.hpp"
#include "pragma/rendering/lighting/c_light_data_buffer_manager.hpp"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/game/game_limits.h"
#include "pragma/console/c_cvar.h"
#include <wgui/types/wirect.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <sharedutils/scope_guard.h>
#include <pragma/entities/entity_iterator.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static void cmd_forwardplus_tile_size(NetworkState *, const ConVar &, int32_t, int32_t val)
{
	if(c_game == NULL)
		return;

	pragma::ShaderForwardPLightCulling::TILE_SIZE = val;
	for(auto &c : EntityCIterator<pragma::CRasterizationRendererComponent> {*c_game}) {
		auto &fp = c.GetForwardPlusInstance();
		auto &prepass = c.GetPrepass();
		c_engine->GetRenderContext().WaitIdle();
		fp.Initialize(c_engine->GetRenderContext(), c.GetWidth(), c.GetHeight(), *prepass.textureDepth);
		auto cRenderer = c.GetRendererComponent();
		if(cRenderer)
			cRenderer->UpdateRenderSettings();
	}

	c_engine->ReloadShader("forwardp_light_culling");
}
REGISTER_CONVAR_CALLBACK_CL(render_forwardplus_tile_size, cmd_forwardplus_tile_size);

static auto cvTileSize = GetClientConVar("render_forwardplus_tile_size");
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
	return umath::to_integral(GameLimits::MaxAbsoluteShadowLights) / 32u + 1u;
}

pragma::rendering::ForwardPlusInstance::ForwardPlusInstance(CRasterizationRendererComponent &rasterizer) : m_rasterizer {rasterizer}
{
	m_cmdBuffer = c_engine->GetRenderContext().AllocatePrimaryLevelCommandBuffer(prosper::QueueFamilyType::Compute, m_cmdBufferQueueFamilyIndex);

	m_shaderLightCulling = c_engine->GetShader("forwardp_light_culling");
}

bool pragma::rendering::ForwardPlusInstance::Initialize(prosper::IPrContext &context, uint32_t width, uint32_t height, prosper::Texture &depthTexture)
{
	if(pragma::ShaderGameWorldLightingPass::DESCRIPTOR_SET_RENDERER.IsValid() == false)
		return false;
	auto workGroupCount = CalcWorkGroupCount(width, height);
	m_workGroupCountX = workGroupCount.first;
	m_workGroupCountY = workGroupCount.second;
	m_tileCount = CalcTileCount(width, height);

	using VisibleIndex = int32_t;
	auto numLights = umath::to_integral(GameLimits::MaxAbsoluteLights);
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

	m_rasterizer.GetRendererDescriptorSet()->SetBindingStorageBuffer(*m_bufTileVisLightIndex, umath::to_integral(pragma::ShaderGameWorldLightingPass::RendererBinding::TileVisLightIndexBuffer));

	auto &descSetCompute = *m_rasterizer.GetLightSourceDescriptorSetCompute();
	descSetCompute.SetBindingStorageBuffer(*m_bufTileVisLightIndex, umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::TileVisLightIndexBuffer));
	descSetCompute.SetBindingTexture(depthTexture, umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::DepthMap));
	descSetCompute.SetBindingStorageBuffer(*m_bufVisLightIndex, umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::VisLightIndexBuffer));
	return true;
}

void pragma::rendering::ForwardPlusInstance::Compute(prosper::IPrimaryCommandBuffer &cmdBuffer, pragma::CSceneComponent &scene, prosper::IImage &imgDepth, prosper::IDescriptorSet &descSetCam)
{
	if(m_shaderLightCulling.expired() || m_shadowLightBits.empty() == true)
		return;
	auto &shaderLightCulling = static_cast<pragma::ShaderForwardPLightCulling &>(*m_shaderLightCulling.get());
	prosper::ShaderBindState bindState {cmdBuffer};
	if(shaderLightCulling.RecordBeginCompute(bindState) == false)
		return;

	auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
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
