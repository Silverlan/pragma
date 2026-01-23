// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.light_data_buffer_manager;

export import :entities.components.lights.light;

export namespace pragma {
#if USE_LIGHT_SOURCE_UNIFORM_BUFFER == 1
	constexpr auto LIGHT_SOURCE_BUFFER_TYPE = prosper::DescriptorType::UniformBuffer;
#else
	constexpr auto LIGHT_SOURCE_BUFFER_TYPE = prosper::DescriptorType::StorageBuffer;
#endif
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
