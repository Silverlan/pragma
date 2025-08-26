// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <wgui/types/wirect.h>

export module pragma.client.gui:icon;

export {
	class DLLCLIENT WIIcon : public WITexturedRect {
	public:
		WIIcon();
		virtual ~WIIcon() override;
		virtual void Initialize() override;
		void SetClipping(uint32_t xStart, uint32_t yStart, uint32_t width, uint32_t height);
	};
};
