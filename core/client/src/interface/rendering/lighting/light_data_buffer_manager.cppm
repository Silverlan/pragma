// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.light_data_buffer_manager;

export import :entities.components.lights.light;

export namespace pragma {
	constexpr auto LIGHT_SOURCE_BUFFER_TYPE = prosper::DescriptorType::StorageBuffer;
	constexpr auto SHADOW_BUFFER_TYPE = prosper::DescriptorType::UniformBuffer;
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
		prosper::InFlightIndexedBuffer &GetGlobalRenderBuffer();
		void WriteBufferData(prosper::InFlightIndexedBuffer::Index index, prosper::IBuffer::Offset offset, prosper::IBuffer::Size size, const void *data);

		void Free(prosper::InFlightIndexedBuffer::Index index);
	  protected:
		BaseLightBufferManager() = default;
		virtual void DoInitialize() = 0;
		std::optional<prosper::InFlightIndexedBuffer::Index> Request(CLightComponent &lightSource, const void *data, size_t dataSize);

		std::shared_ptr<prosper::InFlightIndexedBuffer> m_masterBuffer = nullptr;
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
		std::optional<prosper::InFlightIndexedBuffer::Index> Request(CLightComponent &lightSource, const ShadowBufferData &bufferData);
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

		size_t GetLightDataBufferCount() const;
		std::optional<prosper::InFlightIndexedBuffer::Index> Request(CLightComponent &lightSource, const LightBufferData &bufferData);
		void Reset() override;
	  private:
		LightDataBufferManager() = default;
		void DoInitialize() override;
	};
};
