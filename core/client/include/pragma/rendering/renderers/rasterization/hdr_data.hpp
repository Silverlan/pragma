#ifndef __HDR_DATA_HPP__
#define __HDR_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <memory>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/functioncallback.h>
#include <mathutil/uvec.h>
#include <misc/types.h>

class Scene;
namespace prosper
{
	class Texture;
	class CommandBuffer;
	class PrimaryCommandBuffer;
	class RenderPass;
	class RenderTarget;
	class BlurSet;
	class DescriptorSetGroup;
	class Shader;
	class Buffer;
	class Fence;
};
namespace pragma::rendering
{
	class RasterizationRenderer;
	class DLLCLIENT HDRData
	{
	public:
		HDRData(RasterizationRenderer &rasterizer);
		~HDRData();
		void UpdateExposure();
		bool Initialize(RasterizationRenderer &renderer,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits sampleCount,bool bEnableSSAO);
		bool InitializeDescriptorSets();

		void SwapIOTextures();

		bool BeginRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,prosper::RenderPass *customRenderPass=nullptr);
		bool EndRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);
		bool ResolveRenderPass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd);

		void ResetIOTextureIndex();
		bool BlitStagingRenderTargetToMainRenderTarget(prosper::CommandBuffer &cmdBuffer);
		bool BlitMainDepthBufferToSamplableDepthBuffer(prosper::CommandBuffer &cmdBuffer,std::function<void(prosper::CommandBuffer&)> &fTransitionSampleImgToTransferDst);

		SSAOInfo ssaoInfo;
		pragma::rendering::Prepass prepass;
		pragma::rendering::ForwardPlusInstance forwardPlusInstance;
		std::shared_ptr<prosper::Texture> bloomTexture = nullptr;
		std::shared_ptr<prosper::Texture> bloomBlurTexture = nullptr;
		std::shared_ptr<prosper::RenderTarget> bloomBlurRenderTarget = nullptr;
		std::shared_ptr<prosper::RenderTarget> hdrRenderTarget = nullptr;
		std::shared_ptr<prosper::RenderTarget> hdrStagingRenderTarget = nullptr;
		std::shared_ptr<prosper::RenderTarget> postHdrRenderTarget = nullptr;

		std::shared_ptr<prosper::BlurSet> bloomBlurSet = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupHdr = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupPostHdr = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupHdrResolve = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupHdrResolveStaging = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupDepth = nullptr;
		std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupDepthPostProcessing = nullptr;

		std::shared_ptr<prosper::RenderPass> rpIntermediate = nullptr;

		util::WeakHandle<prosper::Shader> shaderPPHdr = {};

		float exposure;
		float max_exposure;
		std::array<float,3> luminescence;
	private:
		uint32_t m_curTex = 0;
		CallbackHandle m_cbReloadCommandBuffer;
		void InitializeCommandBuffer();
		struct Exposure
		{
			Exposure();
			std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupAverageColorTexture = nullptr;
			std::shared_ptr<prosper::DescriptorSetGroup> descSetGroupAverageColorBuffer = nullptr;
			Vector3 averageColor;
			std::shared_ptr<prosper::Buffer> avgColorBuffer = nullptr;
			double lastExposureUpdate;
			bool Initialize(prosper::Texture &texture);
			const Vector3 &UpdateColor();
		private:
			util::WeakHandle<prosper::Shader> m_shaderCalcColor = {};
			std::weak_ptr<prosper::Texture> m_exposureColorSource = {};
			std::shared_ptr<prosper::PrimaryCommandBuffer> m_calcImgColorCmdBuffer = nullptr;
			std::shared_ptr<prosper::Fence> m_calcImgColorFence = nullptr;
			bool m_bWaitingForResult = false;
			uint32_t m_cmdBufferQueueFamilyIndex = 0u;
		} m_exposure;
		Bool m_bMipmapInitialized;
	};
};

#endif
