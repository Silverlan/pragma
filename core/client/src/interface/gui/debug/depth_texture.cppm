// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.debug_depth_texture;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIDebugDepthTexture : public WIBase {
	  public:
		WIDebugDepthTexture();
		virtual ~WIDebugDepthTexture() override;
		virtual void Initialize() override;

		void SetTexture(prosper::Texture &texture, bool stencil = false);
		void SetTexture(prosper::Texture &texture, prosper::util::BarrierImageLayout srcLayout, prosper::util::BarrierImageLayout dstLayout, uint32_t layerId = 0u, bool stencil = false);

		void SetContrastFactor(float contrastFactor);
		float GetContrastFactor() const;

		void Setup(float nearZ, float farZ);
		void SetShouldResolveImage(bool b);
	  protected:
		virtual void DoUpdate() override;
		void UpdateResolvedTexture();
		WIHandle m_hTextureRect;

		util::WeakHandle<prosper::Shader> m_whDepthToRgbShader = {};
		util::WeakHandle<prosper::Shader> m_whCubeDepthToRgbShader = {};
		util::WeakHandle<prosper::Shader> m_whCsmDepthToRgbShader = {};
		CallbackHandle m_depthToRgbCallback = {};
		std::shared_ptr<prosper::Texture> m_srcDepthTex = nullptr;
		std::shared_ptr<prosper::RenderTarget> m_renderTarget = nullptr;
		prosper::util::BarrierImageLayout m_srcBarrierImageLayout = {};
		prosper::util::BarrierImageLayout m_dstBarrierImageLayout = {};
		std::shared_ptr<prosper::IDescriptorSetGroup> m_dsgSceneDepthTex = nullptr;
		uint32_t m_imageLayer = 0u;
		float m_contrastFactor = 1.f;

		bool m_bResolveImage = true;
	};
};
