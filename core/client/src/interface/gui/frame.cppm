// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:gui.frame;

export import :gui.transformable;

export import pragma.gui;
export import pragma.platform;
export import pragma.string.unicode;

export namespace pragma::gui::types {
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
		pragma::gui::WIHandle m_hBg;
		pragma::gui::WIHandle m_hTitle;
		pragma::gui::WIHandle m_hTitleBar;
		pragma::gui::WIHandle m_hClose;
		pragma::gui::WIHandle m_hDetachButton;
		pragma::gui::WIHandle m_hContents;
		void OnDetachButtonPressed();
	};
};
