// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>
#include <cstddef>

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
prosper::InFlightIndexedBuffer &BaseLightBufferManager::GetGlobalRenderBuffer() { return *m_masterBuffer; }

void BaseLightBufferManager::WriteBufferData(prosper::InFlightIndexedBuffer::Index index, prosper::IBuffer::Offset offset, prosper::IBuffer::Size size, const void *data) { m_masterBuffer->Write(index, offset, size, data); }

std::optional<prosper::InFlightIndexedBuffer::Index> BaseLightBufferManager::Request(CLightComponent &lightSource, const void *data, size_t dataSize)
{
	auto index = m_masterBuffer->Allocate(data);
	if(!index)
		return {};
	m_masterBuffer->Write(*index, 0, dataSize, data);
	m_bufferIndexToLightSource[*index] = &lightSource;
	return index;
}
void BaseLightBufferManager::Free(prosper::InFlightIndexedBuffer::Index index)
{
	m_masterBuffer->Free(index);
	m_bufferIndexToLightSource[index] = nullptr;
}

////////////////////

ShadowDataBufferManager &ShadowDataBufferManager::GetInstance()
{
	static ShadowDataBufferManager s_instance {};
	return s_instance;
}
void ShadowDataBufferManager::DoInitialize()
{
	auto &context = get_cengine()->GetRenderContext();

	prosper::DeviceSize limit = 0;

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::UniformBufferBit;
	limit = math::to_integral(GameLimits::MaxUniformBufferSize);

	constexpr auto shadowDataSize = sizeof(ShadowBufferData);
	auto numShadows = static_cast<uint32_t>(math::min(static_cast<uint64_t>(limit / shadowDataSize), static_cast<uint64_t>(GameLimits::MaxAbsoluteShadowLights)));
	m_maxCount = numShadows;

	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	createInfo.size = m_maxCount * shadowDataSize;
	createInfo.debugName = "light_shadow_data_buf";
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;

	auto alignment = context.CalcBufferAlignment(createInfo.usageFlags);
	createInfo.size = prosper::util::get_aligned_size(createInfo.size, alignment) * context.GetMaxNumberOfFramesInFlight();
	auto baseBuf = context.CreateResizableBuffer(createInfo);
	baseBuf->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);
	m_masterBuffer = prosper::InFlightIndexedBuffer::Create(*baseBuf, shadowDataSize, alignment);

	m_bufferIndexToLightSource.resize(m_maxCount, nullptr);
}
std::optional<prosper::InFlightIndexedBuffer::Index> ShadowDataBufferManager::Request(CLightComponent &lightSource, const ShadowBufferData &bufferData) { return BaseLightBufferManager::Request(lightSource, &bufferData, sizeof(bufferData)); }

////////////////////

LightDataBufferManager &LightDataBufferManager::GetInstance()
{
	static LightDataBufferManager s_instance {};
	return s_instance;
}
void LightDataBufferManager::DoInitialize()
{
	auto &context = get_cengine()->GetRenderContext();
	auto limits = context.GetPhysicalDeviceLimits();

	constexpr auto lightDataSize = sizeof(LightBufferData);
	prosper::DeviceSize maxBufferSize = 0;

	prosper::util::BufferCreateInfo createInfo {};
	createInfo.usageFlags = prosper::BufferUsageFlags::StorageBufferBit;
	maxBufferSize = limits.maxStorageBufferRange;

	auto numLights = static_cast<uint32_t>(math::min(static_cast<uint64_t>(maxBufferSize / lightDataSize), static_cast<uint64_t>(GameLimits::MaxAbsoluteLights)));
	m_maxCount = numLights;
	createInfo.memoryFeatures = prosper::MemoryFeatureFlags::CPUToGPU;
	createInfo.size = m_maxCount * lightDataSize;
	createInfo.flags |= prosper::util::BufferCreateInfo::Flags::Persistent;

	createInfo.debugName = "light_data_buf";
	auto alignment = context.CalcBufferAlignment(createInfo.usageFlags);

	auto baseSize = createInfo.size;
	createInfo.size = prosper::util::get_aligned_size(createInfo.size, alignment) * context.GetMaxNumberOfFramesInFlight();
	auto baseBuf = context.CreateResizableBuffer(createInfo);
	baseBuf->SetPermanentlyMapped(true, prosper::IBuffer::MapFlags::WriteBit | prosper::IBuffer::MapFlags::Unsynchronized);

	std::vector<uint8_t> initialLightBufferData {};
	{
		auto stride = prosper::util::get_aligned_size(lightDataSize, alignment);
		initialLightBufferData.resize(baseSize);
		LightBufferData initial {};
		auto *data = initialLightBufferData.data();
		for(auto i = decltype(m_maxCount) {0u}; i < m_maxCount; ++i) {
			memcpy(data, &initial, sizeof(initial));
			data += stride;
		}
	}

	m_masterBuffer = prosper::InFlightIndexedBuffer::Create(*baseBuf, lightDataSize, alignment, initialLightBufferData.data());

	m_bufferIndexToLightSource.resize(m_maxCount, nullptr);
}
void LightDataBufferManager::Reset() { BaseLightBufferManager::Reset(); }
size_t LightDataBufferManager::GetLightDataBufferCount() const { return m_masterBuffer->GetAllocatedBufferCount(); }
std::optional<prosper::InFlightIndexedBuffer::Index> LightDataBufferManager::Request(CLightComponent &lightSource, const LightBufferData &bufferData) { return BaseLightBufferManager::Request(lightSource, &bufferData, sizeof(bufferData)); }
