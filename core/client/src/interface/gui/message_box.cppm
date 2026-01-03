// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:gui.message_box;

export import pragma.gui;

export namespace pragma::gui::types {
	class DLLCLIENT WIMessageBox : public WIBase {
	  public:
		enum class Button : Int32 { NONE = 0, APPLY = 1, CANCEL = 2, YES = 4, NO = 8, YESNO = 12, ACCEPT = 16, DECLINE = 32, OK = 64 };
	  private:
		static void __buttonCallback(WIHandle hMessageBox, Button button);
	  protected:
		WIHandle m_hBg;
		WIHandle m_hMessage;
		WIHandle m_hText;
		std::vector<WIHandle> m_buttons;
		std::function<void(WIMessageBox *, Button)> m_buttonCallback;
		WIButton *AddButton(const std::string &text, Button button);
		virtual void OnRemove() override;
	  public:
		WIMessageBox();
		virtual ~WIMessageBox() override;
		virtual void Initialize() override;
		virtual void SetSize(int x, int y) override;
		void SetTitle(const std::string &title);
		void SetText(const std::string &text);
		void EnableButtons(Button buttons);
		void SetButtonCallback(const std::function<void(WIMessageBox *, Button)> &callback);
		static void ShowMessageBox(const std::string &text, const std::string &title, Button buttons, const std::function<void(WIMessageBox *)> &onShow, const std::function<void(WIMessageBox *, Button)> &buttonCallback = nullptr);
		static WIMessageBox *Create(const std::string &text, const std::string &title = "", Button buttons = Button::NONE, const std::function<void(WIMessageBox *, Button)> &callback = nullptr);
		static WIMessageBox *Create(const std::string &text, Button buttons, const std::function<void(WIMessageBox *, Button)> &callback = nullptr);
	};
};

export {
	DLLCLIENT pragma::gui::types::WIMessageBox::Button operator|(const pragma::gui::types::WIMessageBox::Button &a, const pragma::gui::types::WIMessageBox::Button &b);
	DLLCLIENT pragma::gui::types::WIMessageBox::Button operator|=(const pragma::gui::types::WIMessageBox::Button &a, const pragma::gui::types::WIMessageBox::Button &b);
	DLLCLIENT pragma::gui::types::WIMessageBox::Button operator&(const pragma::gui::types::WIMessageBox::Button &a, const pragma::gui::types::WIMessageBox::Button &b);
}
