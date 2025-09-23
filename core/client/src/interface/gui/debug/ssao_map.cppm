// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

export module pragma.client:gui.debug_ssao;

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
