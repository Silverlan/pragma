// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.key_entry;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIKeyEntry : public WITextEntryBase {
	  protected:
		std::string m_previousKey;
		platform::Key m_key;
		bool m_bKeyPressed;
		WIHandle m_hMouseTrap;
		virtual void OnTextChanged(const string::Utf8String &text, bool changedByUser) override;
		void ApplyKey(platform::Key key);
	  public:
		WIKeyEntry();
		virtual ~WIKeyEntry() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		virtual util::EventReply KeyboardCallback(platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) override;
		virtual util::EventReply ScrollCallback(Vector2 offset, bool offsetAsPixels = false) override;
		virtual util::EventReply CharCallback(unsigned int c, platform::Modifier mods = platform::Modifier::None) override;
		virtual void OnFocusGained() override;
		virtual void OnFocusKilled() override;
		void SetKey(platform::Key key);
		platform::Key GetKey() const;
	};
};
