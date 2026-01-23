// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.prepass;
// Not yet fully implemented.
// See https://aras-p.info/blog/2012/03/02/2012-theory-for-forward-rendering/ for implementation details
// #define ENABLE_TRANSLUCENT_DEPTH_PREPASS

export import :rendering.draw_scene_info;
export import :rendering.shaders.prepass;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma {
	namespace rendering {
		class DLLCLIENT Prepass {
		  public:
			bool Initialize(prosper::IPrContext &context, uint32_t width, uint32_t height, prosper::SampleCountFlags sampleCount, bool bExtended);
			ShaderPrepassBase &GetShader() const;
			prosper::RenderTarget &BeginRenderPass(const DrawSceneInfo &drawSceneInfo, prosper::IRenderPass *optRenderPass = nullptr, bool secondaryCommandBuffers = false);
			void EndRenderPass(const DrawSceneInfo &drawSceneInfo);
			// Required for SSAO
			std::shared_ptr<prosper::Texture> textureNormals = nullptr;

			std::shared_ptr<prosper::Texture> textureDepth = nullptr;

			// Depth buffer used for sampling (e.g. particle render pass)
			std::shared_ptr<prosper::Texture> textureDepthSampled = nullptr;

			std::shared_ptr<prosper::RenderTarget> renderTarget = nullptr;

#ifdef ENABLE_TRANSLUCENT_DEPTH_PREPASS
			std::shared_ptr<prosper::RenderTarget> renderTargetTranslucent = nullptr;
			std::shared_ptr<prosper::Texture> textureDepthTranslucent = nullptr;
#endif

			std::shared_ptr<prosper::IRenderPass> subsequentRenderPass = nullptr;

			void SetUseExtendedPrepass(bool b, bool bForceReload = false);
			bool IsExtended() const;
		  private:
			bool m_bExtended = false;
			std::vector<prosper::ClearValue> m_clearValues = {};
			util::WeakHandle<prosper::Shader> m_shaderPrepass = {};
		};
	};
};
#pragma warning(pop)
