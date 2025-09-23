// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <wgui/wibase.h>
#include <mathutil/umath.h>

export module pragma.client:gui.slider;

import :gui.progress_bar;

export {
	class DLLCLIENT WISlider : public WIProgressBar {
	private:
		bool m_bMoveSlider;
	public:
		WISlider();
		virtual ~WISlider() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		virtual util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;
		virtual void Think(const std::shared_ptr<prosper::IPrimaryCommandBuffer> &drawCmd) override;

		bool IsBeingDragged() const;
	};
};
