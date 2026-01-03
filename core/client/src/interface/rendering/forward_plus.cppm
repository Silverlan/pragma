// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:rendering.forward_plus;

export import :entities.components.scene;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma {
	namespace rendering {
		class DLLCLIENT ForwardPlusInstance {
		  public:
			ForwardPlusInstance(CRasterizationRendererComponent &rasterizer);
			bool Initialize(prosper::IPrContext &context, uint32_t width, uint32_t height, prosper::Texture &depthTexture);

			std::pair<uint32_t, uint32_t> GetWorkGroupCount() const;
			uint32_t GetTileCount() const;
			const std::vector<uint32_t> &GetShadowLightBits() const;
			prosper::IDescriptorSet *GetDepthDescriptorSetGraphics() const;
			const std::shared_ptr<prosper::IBuffer> &GetTileVisLightIndexBuffer() const;
			const std::shared_ptr<prosper::IBuffer> &GetVisLightIndexBuffer() const;

			void Compute(prosper::IPrimaryCommandBuffer &cmdBuffer, CSceneComponent &scene, prosper::IImage &imgDepth, prosper::IDescriptorSet &descSetCam);

			static std::pair<uint32_t, uint32_t> CalcWorkGroupCount(uint32_t w, uint32_t h);
			static uint32_t CalcTileCount(uint32_t w, uint32_t h);
		  private:
			CRasterizationRendererComponent &m_rasterizer;
			uint32_t m_workGroupCountX = 0u;
			uint32_t m_workGroupCountY = 0u;
			uint32_t m_tileCount = 0u;
			std::vector<uint32_t> m_shadowLightBits;
			std::shared_ptr<prosper::IBuffer> m_bufTileVisLightIndex = nullptr;
			std::shared_ptr<prosper::IBuffer> m_bufVisLightIndex = nullptr;
			util::WeakHandle<prosper::Shader> m_shaderLightCulling = {};
			std::shared_ptr<prosper::IPrimaryCommandBuffer> m_cmdBuffer = nullptr;
			uint32_t m_cmdBufferQueueFamilyIndex = std::numeric_limits<uint32_t>::max();

			std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgSceneDepthBuffer = nullptr;
		};
	};
};
#pragma warning(pop)
