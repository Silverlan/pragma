// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.light_data_buffer_manager;
import :engine;

using namespace pragma;

//#define ENABLE_LIGHT_BUFFER_DEBUGGING
void BaseLightBufferManager::Initialize()
{
	if(m_bInitialized)
		return;
	m_bInitialized = true;
	DoInitialize();
}
void BaseLightBufferManager::Reset()
{
	m_masterBuffer = nullptr;
	m_bufferIndexToLightSource = {};
	m_bInitialized = false;

	m_maxCount = 0u;
}
CLightComponent *BaseLightBufferManager::GetLightByBufferIndex(ShadowBufferIndex idx) { return (idx < m_bufferIndexToLightSource.size()) ? m_bufferIndexToLightSource.at(idx) : nullptr; }
std::size_t BaseLightBufferManager::GetMaxCount() const { return m_maxCount; }
prosper::IUniformResizableBuffer &BaseLightBufferManager::GetGlobalRenderBuffer() { return *m_masterBuffer; }

////////////////////

ShadowDataBufferManager &ShadowDataBufferManager::GetInstance()
{
	static ShadowDataBufferManager s_instance {};
	return s_instance;
}
void ShadowDataBufferManager::DoInitialize()
{
	auto limits = get_cengine()->GetRenderContext().GetPhysicalDeviceLimits();

	prosper::DeviceSize limit = 0;

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	std::optional<uint64_t> alignment {};
#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
	createInfo.usageFlags |= prosper::BufferUsageFlags::UniformBufferBit;
	alignment = sizeof(Vector4);
	limit = math::to_integral(GameLimits::MaxUniformBufferSize);
#else
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
	limit = limits.maxStorageBufferRange;
#endif

	auto shadowDataSize = sizeof(ShadowBufferData);
	auto numShadows = static_cast<uint32_t>(math::min(static_cast<uint64_t>(limit / shadowDataSize), static_cast<uint64_t>(GameLimits::MaxAbsoluteShadowLights)));
	m_maxCount = numShadows;

	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
	createInfo.size = m_maxCount * shadowDataSize;

	m_masterBuffer = get_cengine()->GetRenderContext().CreateUniformResizableBuffer(createInfo, shadowDataSize, createInfo.size, 0.05f, nullptr, alignment);
	m_masterBuffer->SetDebugName("light_shadow_data_buf");

	m_bufferIndexToLightSource.resize(m_maxCount, nullptr);
}
std::shared_ptr<prosper::IBuffer> ShadowDataBufferManager::Request(CLightComponent &lightSource, const ShadowBufferData &bufferData)
{
	auto buf = m_masterBuffer->AllocateBuffer(&bufferData);
	if(buf != nullptr)
		m_bufferIndexToLightSource.at(buf->GetBaseIndex()) = &lightSource;
	return buf;
}
void ShadowDataBufferManager::Free(const std::shared_ptr<prosper::IBuffer> &renderBuffer) { m_bufferIndexToLightSource.at(renderBuffer->GetBaseIndex()) = nullptr; }

////////////////////

LightDataBufferManager &LightDataBufferManager::GetInstance()
{
	static LightDataBufferManager s_instance {};
	return s_instance;
}
void LightDataBufferManager::DoInitialize()
{
	auto limits = get_cengine()->GetRenderContext().GetPhysicalDeviceLimits();

	auto lightDataSize = sizeof(LightBufferData);
	prosper::DeviceSize maxBufferSize = 0;

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::TransferSrcBit | prosper::BufferUsageFlags::TransferDstBit;
	std::optional<uint64_t> alignment {};
#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
	createInfo.usageFlags |= prosper::BufferUsageFlags::UniformBufferBit;
	maxBufferSize = math::to_integral(GameLimits::MaxUniformBufferSize);
	alignment = sizeof(Vector4);
#else
	createInfo.usageFlags |= prosper::BufferUsageFlags::StorageBufferBit;
	maxBufferSize = limits.maxStorageBufferRange;
#endif
	auto numLights = static_cast<uint32_t>(math::min(static_cast<uint64_t>(maxBufferSize / lightDataSize), static_cast<uint64_t>(GameLimits::MaxAbsoluteLights)));
	m_maxCount = numLights;
#ifdef ENABLE_LIGHT_BUFFER_DEBUGGING
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
#else
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUBulk;
#endif
	createInfo.size = m_maxCount * lightDataSize;

	std::vector<uint8_t> initialLightBufferData {};
	{
		auto stride = prosper::util::get_aligned_size(lightDataSize, alignment.has_value() ? *alignment : 0);
		initialLightBufferData.resize(createInfo.size);
		LightBufferData initial {};
		auto *data = initialLightBufferData.data();
		for(auto i = decltype(m_maxCount) {0u}; i < m_maxCount; ++i) {
			memcpy(data, &initial, sizeof(initial));
			data += stride;
		}
	}

	m_masterBuffer = get_cengine()->GetRenderContext().CreateUniformResizableBuffer(createInfo, lightDataSize, createInfo.size, 0.05f, initialLightBufferData.data(), alignment);
	m_masterBuffer->SetDebugName("light_data_buf");

	m_bufferIndexToLightSource.resize(m_maxCount, nullptr);
}
void LightDataBufferManager::Reset()
{
	BaseLightBufferManager::Reset();
	m_lightDataBuffers = {};
	m_highestBufferIndexInUse = std::numeric_limits<uint32_t>::max();
}
uint32_t LightDataBufferManager::GetLightDataBufferCount() const { return std::min(m_lightDataBuffers.size(), static_cast<size_t>(m_highestBufferIndexInUse + 1)); }
std::shared_ptr<prosper::IBuffer> LightDataBufferManager::Request(CLightComponent &lightSource, const LightBufferData &bufferData)
{
	std::shared_ptr<prosper::IBuffer> renderBuffer = nullptr;
	if((m_highestBufferIndexInUse + 1) < m_lightDataBuffers.size() && m_lightDataBuffers.at(m_highestBufferIndexInUse + 1) != nullptr) {
		renderBuffer = m_lightDataBuffers.at(m_highestBufferIndexInUse + 1);
		renderBuffer->Write(0ull, sizeof(bufferData), &bufferData);
	}
	else
		renderBuffer = m_masterBuffer->AllocateBuffer(&bufferData);

	if(renderBuffer == nullptr)
		return nullptr;
	auto baseIndex = renderBuffer->GetBaseIndex();
	m_bufferIndexToLightSource.at(baseIndex) = &lightSource;
	assert(baseIndex >= (m_highestBufferIndexInUse + 1));
	if(baseIndex < (m_highestBufferIndexInUse + 1))
		throw std::logic_error("Light source buffer index " + std::to_string(baseIndex) + " exceeds highest lights buffer index in use (" + std::to_string(m_highestBufferIndexInUse) + ")!");
	m_highestBufferIndexInUse = baseIndex;

	m_lightDataBuffers.resize(m_masterBuffer->GetTotalInstanceCount(), nullptr);
	m_lightDataBuffers.at(baseIndex) = renderBuffer;
	return renderBuffer;
}
void LightDataBufferManager::Free(const std::shared_ptr<prosper::IBuffer> &renderBuffer)
{
	auto baseIndex = renderBuffer->GetBaseIndex();
	m_bufferIndexToLightSource.at(baseIndex) = nullptr;

	if(baseIndex < m_highestBufferIndexInUse) {
		// Swap light source with highest buffer index with this light source
		// to make sure there are no unassigned slots in the lights buffer.
		auto *pLight = m_bufferIndexToLightSource.at(m_highestBufferIndexInUse);
		assert(pLight != nullptr);
		if(pLight == nullptr)
			throw std::logic_error("Expected valid light source at light buffer index " + std::to_string(m_highestBufferIndexInUse) + ", but none available!");
		// TODO: We can just copy the buffer data on the GPU instead
		get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(renderBuffer, 0ull, pLight->GetBufferData());
		pLight->SetRenderBuffer(renderBuffer, false);

		m_bufferIndexToLightSource.at(m_highestBufferIndexInUse--) = nullptr;
		m_bufferIndexToLightSource.at(baseIndex) = pLight;
	}
	else if(baseIndex == m_highestBufferIndexInUse) {
		m_bufferIndexToLightSource.at(m_highestBufferIndexInUse) = nullptr;
		--m_highestBufferIndexInUse;
	}
	else {
		assert(m_highestBufferIndexInUse == 1);
		if(m_highestBufferIndexInUse != 1)
			throw std::logic_error("Light source buffer index " + std::to_string(baseIndex) + " exceeds highest lights buffer index in use (" + std::to_string(m_highestBufferIndexInUse) + ")!");
		m_highestBufferIndexInUse = 0;
		//const auto flags = BufferFlags::None;
		//pragma::get_cengine()->GetRenderContext().ScheduleRecordUpdateBuffer(renderBuffer,offsetof(BufferData,flags),flags);
	}
}
