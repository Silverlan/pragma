// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>
#include <wgui/types/wirect.h>
#include <mathutil/umath.h>
#include <prosper_window.hpp>

export module pragma.client.gui:frame;

import :transformable;
import pragma.platform;

export
{
	class DLLCLIENT WIFrame : public WITransformable {
	  public:
		WIFrame();
		virtual ~WIFrame() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		WIBase *GetContents();
		// using WITransformable::SetSize; // This causes a compiler error on MSVC on the latest version (25-08-29)
		void SetSize(const Vector2i &size) { WITransformable::SetSize(size); }
		void SetTitle(std::string title);
		const pragma::string::Utf8String &GetTitle() const;
		void SetCloseButtonEnabled(bool b);
		void SetDetachButtonEnabled(bool b);
		virtual util::EventReply MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods) override;

		void Detach();
		void Reattach();
		bool IsDetached() const;
	  protected:
		WIHandle m_hBg;
		WIHandle m_hTitle;
		WIHandle m_hTitleBar;
		WIHandle m_hClose;
		WIHandle m_hDetachButton;
		WIHandle m_hContents;
		void OnDetachButtonPressed();
	};
};
