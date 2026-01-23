// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

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
		const string::Utf8String &GetTitle() const;
		void SetCloseButtonEnabled(bool b);
		void SetDetachButtonEnabled(bool b);
		virtual util::EventReply MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods) override;

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
