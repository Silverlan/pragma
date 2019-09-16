#ifndef __C_FORWARDPLUS_HPP__
#define __C_FORWARDPLUS_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <memory>

namespace Anvil
{
	class DescriptorSetGroup;
	class DescriptorSet;
	class Image;
};

namespace prosper
{
	class Context;
	class Buffer;
	class Texture;
	class CommandBuffer;
	class DescriptorSetGroup;
};

#pragma warning(push)
#pragma warning(disable : 4251)
namespace pragma
{
	class CCameraComponent;
	namespace rendering
	{
		class RasterizationRenderer;
		class DLLCLIENT ForwardPlusInstance
		{
		public:
			ForwardPlusInstance(RasterizationRenderer &rasterizer);
			bool Initialize(prosper::Context &context,uint32_t width,uint32_t height,prosper::Texture &depthTexture);

			std::pair<uint32_t,uint32_t> GetWorkGroupCount() const;
			uint32_t GetTileCount() const;
			const std::vector<uint32_t> &GetShadowLightBits() const;
			Anvil::DescriptorSet *GetDescriptorSetGraphics() const;
			Anvil::DescriptorSet *GetDescriptorSetCompute() const;
			Anvil::DescriptorSet *GetDepthDescriptorSetGraphics() const;
			const std::shared_ptr<prosper::Buffer> &GetTileVisLightIndexBuffer() const;
			const std::shared_ptr<prosper::Buffer> &GetVisLightIndexBuffer() const;

			void Compute(prosper::PrimaryCommandBuffer &cmdBuffer,Anvil::Image &imgDepth,Anvil::DescriptorSet &descSetCam);

			static std::pair<uint32_t,uint32_t> CalcWorkGroupCount(uint32_t w,uint32_t h);
			static uint32_t CalcTileCount(uint32_t w,uint32_t h);
		private:
			RasterizationRenderer &m_rasterizer;
			uint32_t m_workGroupCountX = 0u;
			uint32_t m_workGroupCountY = 0u;
			uint32_t m_tileCount = 0u;
			std::vector<uint32_t> m_shadowLightBits;
			std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupLightSourcesGraphics = nullptr;
			std::shared_ptr<prosper::DescriptorSetGroup> m_descSetGroupLightSourcesCompute = nullptr;
			std::shared_ptr<prosper::Buffer> m_bufTileVisLightIndex = nullptr;
			std::shared_ptr<prosper::Buffer> m_bufVisLightIndex = nullptr;
			util::WeakHandle<prosper::Shader> m_shaderLightCulling = {};
			util::WeakHandle<prosper::Shader> m_shaderLightIndexing = {};
			std::shared_ptr<prosper::PrimaryCommandBuffer> m_cmdBuffer = nullptr;
			uint32_t m_cmdBufferQueueFamilyIndex = std::numeric_limits<uint32_t>::max();

			std::shared_ptr<prosper::DescriptorSetGroup> m_dsgSceneDepthBuffer = nullptr;
		};
	};
};
#pragma warning(pop)

#endif
