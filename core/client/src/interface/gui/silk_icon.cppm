// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/converters/gui_element_converter.hpp"

export module pragma.client.gui:silk_icon;

import :icon;

export {
	class DLLCLIENT WISilkIcon : public WIIcon {
	public:
		WISilkIcon();
		virtual ~WISilkIcon() override;
		virtual void Initialize() override;
		void SetIcon(std::string icon);
	};
};
