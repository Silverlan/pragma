#ifndef __C_LIGHT_DATA_BUFFER_MANAGER_HPP__
#define __C_LIGHT_DATA_BUFFER_MANAGER_HPP__

#include "pragma/clientdefinitions.h"
#include <buffers/prosper_uniform_resizable_buffer.hpp>
#include <mathutil/uvec.h>
#include <vector>
#include <cinttypes>
#include <memory>

namespace pragma
{
	class CLightComponent;

	class BaseLightBufferManager
	{
	public:
		BaseLightBufferManager(const BaseLightBufferManager&)=delete;
		BaseLightBufferManager(BaseLightBufferManager&&)=delete;
		BaseLightBufferManager &operator=(const BaseLightBufferManager&)=delete;
		BaseLightBufferManager &operator=(BaseLightBufferManager&&)=delete;

		void Initialize();
		virtual void Reset();
		CLightComponent *GetLightByBufferIndex(uint32_t idx);
		std::size_t GetMaxCount() const;
		const prosper::UniformResizableBuffer &GetGlobalRenderBuffer();
	protected:
		BaseLightBufferManager()=default;
		virtual void DoInitialize()=0;

		std::shared_ptr<prosper::UniformResizableBuffer> m_masterBuffer = nullptr;
		std::vector<CLightComponent*> m_bufferIndexToLightSource;
		std::size_t m_maxCount = 0u;
	private:
		bool m_bInitialized = false;
	};

	class ShadowDataBufferManager
		: public BaseLightBufferManager
	{
	public:
		ShadowDataBufferManager(const ShadowDataBufferManager&)=delete;
		ShadowDataBufferManager(ShadowDataBufferManager&&)=delete;
		ShadowDataBufferManager &operator=(const ShadowDataBufferManager&)=delete;
		ShadowDataBufferManager &operator=(ShadowDataBufferManager&&)=delete;
		static ShadowDataBufferManager &GetInstance();

		std::shared_ptr<prosper::Buffer> Request(CLightComponent &lightSource,const ShadowBufferData &bufferData);
		void Free(const std::shared_ptr<prosper::Buffer> &renderBuffer);
	private:
		ShadowDataBufferManager()=default;
		virtual void DoInitialize() override;
	};

	////////////////////

	class LightDataBufferManager
		: public BaseLightBufferManager
	{
	public:
		LightDataBufferManager(const LightDataBufferManager&)=delete;
		LightDataBufferManager(LightDataBufferManager&&)=delete;
		LightDataBufferManager &operator=(const LightDataBufferManager&)=delete;
		LightDataBufferManager &operator=(LightDataBufferManager&&)=delete;
		static LightDataBufferManager &GetInstance();

		std::shared_ptr<prosper::Buffer> Request(CLightComponent &lightSource,const LightBufferData &bufferData);
		void Free(const std::shared_ptr<prosper::Buffer> &renderBuffer);
		virtual void Reset() override;
	private:
		LightDataBufferManager()=default;
		virtual void DoInitialize() override;
		std::vector<std::shared_ptr<prosper::Buffer>> m_lightDataBuffers {}; // Sub-buffers allocated from master buffer
		uint32_t m_highestBufferIndexInUse = std::numeric_limits<uint32_t>::max();
	};
};

#endif
