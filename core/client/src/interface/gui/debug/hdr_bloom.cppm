// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"


export module pragma.client:gui.debug_hdr_bloom;

import pragma.gui;

export {
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
