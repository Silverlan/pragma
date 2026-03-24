// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.key_entry;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIKeyEntry : public WITextEntryBase {
	  public:
		WIKeyEntry();
		virtual ~WIKeyEntry() override;
		virtual void Initialize() override;
		virtual util::EventReply KeyboardCallback(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) override;
		virtual util::EventReply ScrollCallback(Vector2 offset, bool offsetAsPixels = false) override;
		virtual util::EventReply CharCallback(unsigned int c, platform::Modifier mods = platform::Modifier::None) override;
		virtual void OnFocusGained() override;
		virtual void OnFocusKilled() override;
		void SetKey(platform::Key key);
		platform::Key GetKey() const;
	  protected:
		std::string m_previousKey;
		platform::Key m_key;
		bool m_bKeyPressed;
		WIHandle m_hMouseTrap;
		virtual void OnTextChanged(const string::Utf8String &text, bool changedByUser) override;
	    virtual void OnSizeChanged(const Vector2i &oldSize, ChangeSource changeSource) override;
		void ApplyKey(platform::Key key);
	};
};
