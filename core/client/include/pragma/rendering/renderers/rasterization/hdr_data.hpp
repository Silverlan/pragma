#ifndef __HDR_DATA_HPP__
#define __HDR_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/c_prepass.hpp"
#include "pragma/rendering/c_forwardplus.hpp"
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

		// This is the render target for the lighting pass, containing the
		// 1) color image (HDR)
		// 2) bloom color image (HDR), containing all bright colors
		// 3) depth image
		std::shared_ptr<prosper::RenderTarget> sceneRenderTarget = nullptr;
		// Bound to HDR color image; Used for HDR post-processing
		std::shared_ptr<prosper::DescriptorSetGroup> dsgHDRPostProcessing = nullptr;

		// Contains the bright colors of the scene, as output by the lighting pass
		std::shared_ptr<prosper::Texture> bloomTexture = nullptr;

		// Contains the blurred bright areas, is overlayed over the scene in post-processing
		std::shared_ptr<prosper::RenderTarget> bloomBlurRenderTarget = nullptr;
		std::shared_ptr<prosper::BlurSet> bloomBlurSet = nullptr;

		// Bound to HDR color image and HDR blurred bloom color image, used for tonemapping and
		// applying bloom effect
		std::shared_ptr<prosper::DescriptorSetGroup> dsgBloomTonemapping = nullptr;

		// Render target for post-processing after the lighting pass with HDR colors
		std::shared_ptr<prosper::RenderTarget> hdrPostProcessingRenderTarget = nullptr;

		// Render target containing image after tonemapping
		std::shared_ptr<prosper::RenderTarget> toneMappedRenderTarget = nullptr;
		// Bound to tonemapped (LDR) color image
		std::shared_ptr<prosper::DescriptorSetGroup> dsgTonemappedPostProcessing = nullptr;

		// Render target for post-processing after tonemapping
		std::shared_ptr<prosper::RenderTarget> toneMappedPostProcessingRenderTarget = nullptr;
		// Bound to tonemapped post-processing image
		std::shared_ptr<prosper::DescriptorSetGroup> dsgToneMappedPostProcessing = nullptr;

		// Bound to depth image of lighting stage; Used for particle effects
		std::shared_ptr<prosper::DescriptorSetGroup> dsgSceneDepth = nullptr;
		// Bound to post-scene depth image; Used for post-processing (e.g. fog)
		std::shared_ptr<prosper::DescriptorSetGroup> dsgDepthPostProcessing = nullptr;

		// Render pass used to restart scene pass after particle pass
		std::shared_ptr<prosper::RenderPass> rpPostParticle = nullptr;

		float exposure = 1.f;
		float max_exposure = 1,f;
		std::array<float,3> luminescence = {0.f,0.f,0.f};
	private:
		uint32_t m_curTex = 0;
		CallbackHandle m_cbReloadCommandBuffer;
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
