#include "stdafx_client.h"
#include "pragma/rendering/c_forwardplus.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_culling.hpp"
#include "pragma/rendering/shaders/c_shader_forwardp_light_indexing.hpp"
#include <wgui/types/wirect.h>
#include <prosper_util.hpp>
#include <buffers/prosper_buffer.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <sharedutils/scope_guard.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

std::pair<uint32_t,uint32_t> pragma::rendering::ForwardPlusInstance::CalcWorkGroupCount(uint32_t w,uint32_t h)
{
	const auto tileSize = pragma::ShaderForwardPLightCulling::TILE_SIZE;
	const auto fCalcWorkGroups = [tileSize](uint32_t v) {
		auto numGroups = v;
		if((numGroups %tileSize) == 0)
			numGroups /= tileSize;
		else
			numGroups = (numGroups +(tileSize -(numGroups %tileSize))) /tileSize;
		return numGroups;
	};

	return {fCalcWorkGroups(w),fCalcWorkGroups(h)};
}
uint32_t pragma::rendering::ForwardPlusInstance::CalcTileCount(uint32_t w,uint32_t h)
{
	auto workGroupCount = CalcWorkGroupCount(w,h);
	return workGroupCount.first *workGroupCount.second;
}

static constexpr uint32_t get_shadow_integer_count()
{
	// Minimum number of integers required to contain 1 bit for each light with a shadow
	return umath::to_integral(GameLimits::MaxAbsoluteShadowLights) /32u +1u;
}

pragma::rendering::ForwardPlusInstance::ForwardPlusInstance()
{
	m_cmdBuffer = c_engine->AllocatePrimaryLevelCommandBuffer(Anvil::QueueFamilyType::COMPUTE,m_cmdBufferQueueFamilyIndex);

	m_shaderLightCulling = c_engine->GetShader("forwardp_light_culling");
	m_shaderLightIndexing = c_engine->GetShader("forwardp_light_indexing");
}

bool pragma::rendering::ForwardPlusInstance::Initialize(prosper::Context &context,uint32_t width,uint32_t height,prosper::Texture &depthTexture,const Camera &cam)
{
	if(pragma::ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS.IsValid() == false)
		return false;
	auto &dev = context.GetDevice();
	auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
	m_descSetGroupLightSourcesGraphics = prosper::util::create_descriptor_set_group(dev,pragma::ShaderTextured3DBase::DESCRIPTOR_SET_LIGHTS);
	prosper::util::set_descriptor_set_binding_storage_buffer(
		*(*m_descSetGroupLightSourcesGraphics)->get_descriptor_set(0u),const_cast<prosper::UniformResizableBuffer&>(bufLightSources),umath::to_integral(pragma::ShaderTextured3DBase::LightBinding::LightBuffers)
	);
	prosper::util::set_descriptor_set_binding_storage_buffer(
		*(*m_descSetGroupLightSourcesGraphics)->get_descriptor_set(0u),const_cast<prosper::UniformResizableBuffer&>(bufShadowData),umath::to_integral(pragma::ShaderTextured3DBase::LightBinding::ShadowData)
	);

	m_descSetGroupLightSourcesCompute = prosper::util::create_descriptor_set_group(dev,pragma::ShaderForwardPLightCulling::DESCRIPTOR_SET_LIGHTS);
	prosper::util::set_descriptor_set_binding_storage_buffer(
		*(*m_descSetGroupLightSourcesCompute)->get_descriptor_set(0u),const_cast<prosper::UniformResizableBuffer&>(bufLightSources),umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::LightBuffers)
	);
	prosper::util::set_descriptor_set_binding_storage_buffer(
		*(*m_descSetGroupLightSourcesCompute)->get_descriptor_set(0u),const_cast<prosper::UniformResizableBuffer&>(bufShadowData),umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::ShadowData)
	);

	auto workGroupCount = CalcWorkGroupCount(width,height);
	m_workGroupCountX = workGroupCount.first;
	m_workGroupCountY = workGroupCount.second;
	m_tileCount = CalcTileCount(width,height);

	using VisibleIndex = int32_t;
	auto numLights = umath::to_integral(GameLimits::MaxAbsoluteLights);
	auto numTiles = m_tileCount *numLights;

	std::vector<VisibleIndex> defaultIndices(numTiles,-1);
	prosper::util::BufferCreateInfo createInfo {};
	createInfo.size = defaultIndices.size() *sizeof(defaultIndices.front());
	createInfo.usageFlags = Anvil::BufferUsageFlagBits::STORAGE_BUFFER_BIT;
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUBulk;
	m_bufTileVisLightIndex = prosper::util::create_buffer(dev,createInfo,defaultIndices.data());
	m_bufTileVisLightIndex->SetDebugName("tile_vis_light_index_buf");

	m_shadowLightBits.resize(get_shadow_integer_count(),0);
	createInfo.size = m_shadowLightBits.size() *sizeof(m_shadowLightBits.front());
	createInfo.memoryFeatures = prosper::util::MemoryFeatureFlags::GPUToCPU;
	m_bufVisLightIndex = prosper::util::create_buffer(dev,createInfo,m_shadowLightBits.data());
	m_bufVisLightIndex->SetPermanentlyMapped(true);
	m_bufVisLightIndex->SetDebugName("vis_light_index_buf");
	
	prosper::util::set_descriptor_set_binding_storage_buffer(
		*(*m_descSetGroupLightSourcesGraphics)->get_descriptor_set(0u),*m_bufTileVisLightIndex,umath::to_integral(pragma::ShaderTextured3DBase::LightBinding::TileVisLightIndexBuffer)
	);

	auto &descSetCompute = *(*m_descSetGroupLightSourcesCompute)->get_descriptor_set(0u);
	prosper::util::set_descriptor_set_binding_storage_buffer(
		descSetCompute,*m_bufTileVisLightIndex,umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::TileVisLightIndexBuffer)
	);
	prosper::util::set_descriptor_set_binding_texture(descSetCompute,depthTexture,umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::DepthMap));
	prosper::util::set_descriptor_set_binding_storage_buffer(
		descSetCompute,*m_bufVisLightIndex,umath::to_integral(pragma::ShaderForwardPLightCulling::LightBinding::VisLightIndexBuffer)
	);
	return true;
}

void pragma::rendering::ForwardPlusInstance::Compute(prosper::PrimaryCommandBuffer &cmdBuffer,Anvil::Image &imgDepth,Anvil::DescriptorSet &descSetCam)
{
	if(m_shaderLightCulling.expired() || m_shaderLightIndexing.expired() || m_shadowLightBits.empty() == true)
		return;
	auto &shaderLightCulling = static_cast<pragma::ShaderForwardPLightCulling&>(*m_shaderLightCulling.get());
	if(shaderLightCulling.BeginCompute(std::static_pointer_cast<prosper::PrimaryCommandBuffer>(cmdBuffer.shared_from_this())) == false)
		return;

	auto &bufLightSources = pragma::CLightComponent::GetGlobalRenderBuffer();
	auto &bufShadowData = pragma::CLightComponent::GetGlobalShadowBuffer();
	// Light source data barrier
	prosper::util::record_buffer_barrier(
		*cmdBuffer,bufLightSources,
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT | Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::TRANSFER_BIT,
		Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::TRANSFER_WRITE_BIT
	);
	// Shadow data barrier
	prosper::util::record_buffer_barrier(
		*cmdBuffer,bufShadowData,
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT | Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT | Anvil::PipelineStageFlagBits::TRANSFER_BIT,Anvil::PipelineStageFlagBits::TRANSFER_BIT,
		Anvil::AccessFlagBits::SHADER_READ_BIT | Anvil::AccessFlagBits::TRANSFER_WRITE_BIT,Anvil::AccessFlagBits::TRANSFER_WRITE_BIT
	);
	
	// Visible light tile index buffer
	prosper::util::record_buffer_barrier(
		*cmdBuffer,*c_game->GetRenderScene()->GetForwardPlusInstance().GetTileVisLightIndexBuffer(),
		Anvil::PipelineStageFlagBits::FRAGMENT_SHADER_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::AccessFlagBits::SHADER_READ_BIT,Anvil::AccessFlagBits::SHADER_WRITE_BIT
	);

	// Visible light index buffer
	prosper::util::record_buffer_barrier(
		*cmdBuffer,*c_game->GetRenderScene()->GetForwardPlusInstance().GetVisLightIndexBuffer(),
		Anvil::PipelineStageFlagBits::HOST_BIT,Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,
		Anvil::AccessFlagBits::HOST_READ_BIT,Anvil::AccessFlagBits::SHADER_WRITE_BIT
	);

	auto workGroupCount = GetWorkGroupCount();
	if(shaderLightCulling.Compute(*GetDescriptorSetCompute(),descSetCam,workGroupCount.first,workGroupCount.second,pragma::CLightComponent::GetLightCount()) == false)
		return;

	// Visible light index buffer
	prosper::util::record_buffer_barrier(
		*cmdBuffer,*c_game->GetRenderScene()->GetForwardPlusInstance().GetVisLightIndexBuffer(),
		Anvil::PipelineStageFlagBits::COMPUTE_SHADER_BIT,Anvil::PipelineStageFlagBits::HOST_BIT,
		Anvil::AccessFlagBits::SHADER_WRITE_BIT,Anvil::AccessFlagBits::HOST_READ_BIT
	);

	shaderLightCulling.EndCompute();

	// TODO: Synchronize this somehow
	const auto szRead = m_shadowLightBits.size() *sizeof(m_shadowLightBits.front());
	m_bufVisLightIndex->Read(0ull,szRead,m_shadowLightBits.data());
}

const std::vector<uint32_t> &pragma::rendering::ForwardPlusInstance::GetShadowLightBits() const {return m_shadowLightBits;}
std::pair<uint32_t,uint32_t> pragma::rendering::ForwardPlusInstance::GetWorkGroupCount() const {return {m_workGroupCountX,m_workGroupCountY};}
uint32_t pragma::rendering::ForwardPlusInstance::GetTileCount() const {return m_tileCount;}
Anvil::DescriptorSet *pragma::rendering::ForwardPlusInstance::GetDescriptorSetGraphics() const {return (*m_descSetGroupLightSourcesGraphics)->get_descriptor_set(0u);}
Anvil::DescriptorSet *pragma::rendering::ForwardPlusInstance::GetDescriptorSetCompute() const {return (*m_descSetGroupLightSourcesCompute)->get_descriptor_set(0u);}
Anvil::DescriptorSet *pragma::rendering::ForwardPlusInstance::GetDepthDescriptorSetGraphics() const {return (*m_descSetGroupDepthBuffer)->get_descriptor_set(0u);}
const std::shared_ptr<prosper::Buffer> &pragma::rendering::ForwardPlusInstance::GetTileVisLightIndexBuffer() const {return m_bufTileVisLightIndex;}
const std::shared_ptr<prosper::Buffer> &pragma::rendering::ForwardPlusInstance::GetVisLightIndexBuffer() const {return m_bufVisLightIndex;}
