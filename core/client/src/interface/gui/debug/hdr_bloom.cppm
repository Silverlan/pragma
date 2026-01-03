// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.debug_hdr_bloom;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIDebugHDRBloom : public WITexturedRect {
	  public:
		WIDebugHDRBloom();
		virtual ~WIDebugHDRBloom() override;
	  private:
		virtual void DoUpdate() override;
		void UpdateBloomImage();
		CallbackHandle m_cbRenderHDRMap = {};
		std::shared_ptr<prosper::RenderTarget> m_renderTarget = nullptr;
	};
};
