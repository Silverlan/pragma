/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __HDR_DATA_HPP__
#define __HDR_DATA_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_ssao.hpp"
#include "pragma/rendering/c_prepass.hpp"
#include "pragma/rendering/c_forwardplus.hpp"
#include "pragma/rendering/scene/util_draw_scene_info.hpp"
#include <cinttypes>
#include <memory>
#include <sharedutils/util_weak_handle.hpp>
#include <sharedutils/functioncallback.h>
#include <mathutil/uvec.h>

class Scene;
namespace prosper {
	class Texture;
	class ICommandBuffer;
	class IPrimaryCommandBuffer;
	class IRenderPass;
	class RenderTarget;
	class BlurSet;
	class IDescriptorSetGroup;
	class Shader;
	class IBuffer;
	class IFence;
};
namespace util {
	struct DrawSceneInfo;
};
namespace pragma {
	class CRasterizationRendererComponent;
};
namespace pragma::rendering {
	class DLLCLIENT HDRData {
	  public:
		HDRData(CRasterizationRendererComponent &rasterizer);
		~HDRData();
		void UpdateExposure();
		bool Initialize(uint32_t width, uint32_t height, prosper::SampleCountFlags sampleCount, bool bEnableSSAO);
		bool InitializeDescriptorSets();
		prosper::RenderTarget &GetRenderTarget(const util::DrawSceneInfo &drawSceneInfo);

		void SwapIOTextures();

		bool BeginRenderPass(const util::DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *customRenderPass = nullptr, bool secondaryCommandBuffers = false);
		bool EndRenderPass(const util::DrawSceneInfo &drawSceneInfo);
		bool ResolveRenderPass(const util::DrawSceneInfo &drawSceneInfo);
		void ReloadPresentationRenderTarget(uint32_t width, uint32_t height, prosper::SampleCountFlags sampleCount);
		bool ReloadBloomRenderTarget(uint32_t width);

		void ResetIOTextureIndex();
		bool BlitStagingRenderTargetToMainRenderTarget(const util::DrawSceneInfo &drawSceneInfo, prosper::ImageLayout srcHdrLayout = prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout dstHdrLayout = prosper::ImageLayout::ColorAttachmentOptimal);
		bool BlitMainDepthBufferToSamplableDepthBuffer(const util::DrawSceneInfo &drawSceneInfo, std::function<void(prosper::ICommandBuffer &)> &fTransitionSampleImgToTransferDst);

		SSAOInfo ssaoInfo;
		pragma::rendering::Prepass prepass;
		pragma::rendering::ForwardPlusInstance forwardPlusInstance;

		// This is the render target for the lighting pass, containing the
		// 1) color image (HDR)
		// 2) bloom color image (HDR), containing all bright colors
		// 3) depth image
		std::shared_ptr<prosper::RenderTarget> sceneRenderTarget = nullptr;
		// Bound to HDR color image; Used for HDR post-processing
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgHDRPostProcessing = nullptr;

		// Contains the bright colors of the scene, as output by the lighting pass
		std::shared_ptr<prosper::Texture> bloomTexture = nullptr;

		// Contains the blurred bright areas, is overlayed over the scene in post-processing
		std::shared_ptr<prosper::RenderTarget> bloomBlurRenderTarget = nullptr;
		std::shared_ptr<prosper::BlurSet> bloomBlurSet = nullptr;

		// Bound to HDR color image and HDR blurred bloom color image, used for tonemapping and
		// applying bloom effect
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgBloomTonemapping = nullptr;

		// Render target for post-processing after the lighting pass with HDR colors
		std::shared_ptr<prosper::RenderTarget> hdrPostProcessingRenderTarget = nullptr;

		// Render target containing image after tonemapping
		std::shared_ptr<prosper::RenderTarget> toneMappedRenderTarget = nullptr;
		// Bound to tonemapped (LDR) color image
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgTonemappedPostProcessing = nullptr;

		// Render target for post-processing after tonemapping
		std::shared_ptr<prosper::RenderTarget> toneMappedPostProcessingRenderTarget = nullptr;
		// Bound to tonemapped post-processing image
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgToneMappedPostProcessing = nullptr;

		// Bound to depth image of lighting stage; Used for particle effects
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgSceneDepth = nullptr;
		// Bound to post-scene depth image; Used for post-processing (e.g. fog)
		std::shared_ptr<prosper::IDescriptorSetGroup> dsgDepthPostProcessing = nullptr;

		// Render pass used to restart scene pass after particle pass
		std::shared_ptr<prosper::IRenderPass> rpPostParticle = nullptr;

		// Render target used for particle render pass
		std::shared_ptr<prosper::RenderTarget> rtParticle = nullptr;

		float exposure = 1.f;
		float max_exposure = 1, f;
		std::array<float, 3> luminescence = {0.f, 0.f, 0.f};
	  private:
		static prosper::util::SamplerCreateInfo GetSamplerCreateInfo();
		uint32_t m_curTex = 0;
		CallbackHandle m_cbReloadCommandBuffer;
		struct Exposure {
			Exposure();
			std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupAverageColorTexture = nullptr;
			std::shared_ptr<prosper::IDescriptorSetGroup> descSetGroupAverageColorBuffer = nullptr;
			Vector3 averageColor;
			std::shared_ptr<prosper::IBuffer> avgColorBuffer = nullptr;
			double lastExposureUpdate;
			bool Initialize(prosper::Texture &texture);
			const Vector3 &UpdateColor();
		  private:
			util::WeakHandle<prosper::Shader> m_shaderCalcColor = {};
			std::weak_ptr<prosper::Texture> m_exposureColorSource = {};
			std::shared_ptr<prosper::IPrimaryCommandBuffer> m_calcImgColorCmdBuffer = nullptr;
			std::shared_ptr<prosper::IFence> m_calcImgColorFence = nullptr;
			bool m_bWaitingForResult = false;
			uint32_t m_cmdBufferQueueFamilyIndex = 0u;
		} m_exposure;
		Bool m_bMipmapInitialized;
	};
};

#endif
