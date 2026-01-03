// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.debug_msaa_texture;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIDebugMSAATexture : public WIBase {
	  public:
		WIDebugMSAATexture();
		virtual void Initialize() override;
		void SetTexture(prosper::Texture &texture);

		// If set to false, all sub-images contained in the multi-sampled image will be displayed
		void SetShouldResolveImage(bool b);
	  protected:
		virtual void DoUpdate() override;
		void UpdateResolvedTexture();
		WIHandle m_hTextureRect;

		std::shared_ptr<prosper::Texture> m_msaaTexture = nullptr;
		std::shared_ptr<prosper::Texture> m_resolvedTexture = nullptr;
		std::shared_ptr<prosper::IDescriptorSetGroup> m_descSetGroupImg = nullptr;

		bool m_bShouldResolveImage = true;
	};
};
