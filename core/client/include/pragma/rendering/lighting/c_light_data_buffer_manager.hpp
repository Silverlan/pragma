/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LIGHT_DATA_BUFFER_MANAGER_HPP__
#define __C_LIGHT_DATA_BUFFER_MANAGER_HPP__

#include "pragma/clientdefinitions.h"
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <mathutil/uvec.h>
#include <vector>
#include <cinttypes>
#include <memory>

#define USE_LIGHT_SOURCE_UNIFORM_BUFFER 1

#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
#define LIGHT_SOURCE_BUFFER_TYPE prosper::DescriptorType::UniformBuffer
#else
#define LIGHT_SOURCE_BUFFER_TYPE prosper::DescriptorType::StorageBuffer
#endif

namespace pragma {
	class CLightComponent;
	struct ShadowBufferData;
	struct LightBufferData;

	using LightBufferIndex = uint32_t;
	using ShadowBufferIndex = uint32_t;
	class BaseLightBufferManager {
	  public:
		BaseLightBufferManager(const BaseLightBufferManager &) = delete;
		BaseLightBufferManager(BaseLightBufferManager &&) = delete;
		BaseLightBufferManager &operator=(const BaseLightBufferManager &) = delete;
		BaseLightBufferManager &operator=(BaseLightBufferManager &&) = delete;

		void Initialize();
		virtual void Reset();
		CLightComponent *GetLightByBufferIndex(LightBufferIndex idx);
		std::size_t GetMaxCount() const;
		prosper::IUniformResizableBuffer &GetGlobalRenderBuffer();
	  protected:
		BaseLightBufferManager() = default;
		virtual void DoInitialize() = 0;

		std::shared_ptr<prosper::IUniformResizableBuffer> m_masterBuffer = nullptr;
		std::vector<CLightComponent *> m_bufferIndexToLightSource;
		std::size_t m_maxCount = 0u;
	  private:
		bool m_bInitialized = false;
	};

	class ShadowDataBufferManager : public BaseLightBufferManager {
	  public:
		ShadowDataBufferManager(const ShadowDataBufferManager &) = delete;
		ShadowDataBufferManager(ShadowDataBufferManager &&) = delete;
		ShadowDataBufferManager &operator=(const ShadowDataBufferManager &) = delete;
		ShadowDataBufferManager &operator=(ShadowDataBufferManager &&) = delete;
		static ShadowDataBufferManager &GetInstance();

		CLightComponent *GetLightByBufferIndex(ShadowBufferIndex idx) { return BaseLightBufferManager::GetLightByBufferIndex(idx); }
		std::shared_ptr<prosper::IBuffer> Request(CLightComponent &lightSource, const ShadowBufferData &bufferData);
		void Free(const std::shared_ptr<prosper::IBuffer> &renderBuffer);
	  private:
		ShadowDataBufferManager() = default;
		virtual void DoInitialize() override;
	};

	////////////////////

	class LightDataBufferManager : public BaseLightBufferManager {
	  public:
		LightDataBufferManager(const LightDataBufferManager &) = delete;
		LightDataBufferManager(LightDataBufferManager &&) = delete;
		LightDataBufferManager &operator=(const LightDataBufferManager &) = delete;
		LightDataBufferManager &operator=(LightDataBufferManager &&) = delete;
		static LightDataBufferManager &GetInstance();

		std::shared_ptr<prosper::IBuffer> Request(CLightComponent &lightSource, const LightBufferData &bufferData);
		void Free(const std::shared_ptr<prosper::IBuffer> &renderBuffer);
		virtual void Reset() override;
		uint32_t GetLightDataBufferCount() const;
	  private:
		LightDataBufferManager() = default;
		virtual void DoInitialize() override;
		std::vector<std::shared_ptr<prosper::IBuffer>> m_lightDataBuffers {}; // Sub-buffers allocated from master buffer
		uint32_t m_highestBufferIndexInUse = std::numeric_limits<uint32_t>::max();
	};
};

#endif
