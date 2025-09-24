// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"

export module pragma.client:gui.debug_ssao;

import pragma.gui;

export {
	class DLLCLIENT WIDebugSSAO : public WITexturedRect {
	public:
		WIDebugSSAO();
		void SetUseBlurredSSAOImage(bool b);
	private:
		virtual void DoUpdate() override;
		bool m_bUseBlurVariant = false;
	};
};
