// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.message_box;
import :gui.frame;

import pragma.gui;

const Int32 MARGIN_X = 32;
const Int32 MARGIN_Y = 50;

pragma::gui::types::WIMessageBox *pragma::gui::types::WIMessageBox::Create(const std::string &text, const std::string &title, Button buttons, const std::function<void(WIMessageBox *, Button)> &callback)
{
	auto *pMessageBox = WGUI::GetInstance().Create<WIMessageBox>();
	if(pMessageBox == nullptr)
		return nullptr;
	pMessageBox->SetTitle(title);
	pMessageBox->EnableButtons(buttons);
	pMessageBox->SetAutoAlignToParent(true);
	pMessageBox->SetText(text);
	pMessageBox->SetZPos(10000);
	pMessageBox->SetButtonCallback(callback);
	pMessageBox->TrapFocus(true);
	pMessageBox->RequestFocus();
	return pMessageBox;
}

pragma::gui::types::WIMessageBox *pragma::gui::types::WIMessageBox::Create(const std::string &text, Button buttons, const std::function<void(WIMessageBox *, Button)> &callback) { return Create(text, "", buttons, callback); }

struct MessageBoxDesc {
	std::string text;
	std::string title;
	pragma::gui::types::WIMessageBox::Button buttons;
	std::function<void(pragma::gui::types::WIMessageBox *)> onShow;
	std::function<void(pragma::gui::types::WIMessageBox *, pragma::gui::types::WIMessageBox::Button)> buttonCallback;
};
static std::queue<MessageBoxDesc> s_messageBoxQueue;
static pragma::gui::WIHandle s_currentMessageBox;
static void show_next_message_box()
{
	if(s_messageBoxQueue.empty())
		return;
	auto &desc = s_messageBoxQueue.front();
	auto onShow = desc.onShow;
	auto *msgBox = pragma::gui::types::WIMessageBox::Create(desc.text, desc.title, desc.buttons, desc.buttonCallback);
	s_messageBoxQueue.pop();
	if(msgBox == nullptr)
		return;
	if(onShow != nullptr)
		onShow(msgBox);
	s_currentMessageBox = msgBox->GetHandle();
}
void pragma::gui::types::WIMessageBox::ShowMessageBox(const std::string &text, const std::string &title, Button buttons, const std::function<void(WIMessageBox *)> &onShow, const std::function<void(WIMessageBox *, Button)> &buttonCallback)
{
	MessageBoxDesc desc {};
	desc.text = text;
	desc.title = title;
	desc.buttons = buttons;
	desc.onShow = onShow;
	desc.buttonCallback = buttonCallback;
	s_messageBoxQueue.push(desc);
	if(s_currentMessageBox.IsValid() == false)
		show_next_message_box();
}

pragma::gui::types::WIMessageBox::WIMessageBox() : WIBase() {}
pragma::gui::types::WIMessageBox::~WIMessageBox() {}

void pragma::gui::types::WIMessageBox::OnRemove()
{
	if(s_currentMessageBox.get() == this)
		show_next_message_box();
}

void pragma::gui::types::WIMessageBox::SetTitle(const std::string &title)
{
	if(!m_hMessage.IsValid())
		return;
	static_cast<WIFrame *>(m_hMessage.get())->SetTitle(title);
}

void pragma::gui::types::WIMessageBox::__buttonCallback(WIHandle hMessageBox, Button button)
{
	if(!hMessageBox.IsValid())
		return;
	auto *pMessageBox = static_cast<WIMessageBox *>(hMessageBox.get());
	if(pMessageBox->m_buttonCallback == nullptr)
		return;
	pMessageBox->m_buttonCallback(pMessageBox, button);
};
pragma::gui::types::WIButton *pragma::gui::types::WIMessageBox::AddButton(const std::string &text, Button button)
{
	return nullptr; // TODO
}

void pragma::gui::types::WIMessageBox::EnableButtons(Button buttons)
{
	auto values = math::get_power_of_2_values(CUInt64(buttons));
	auto hMessageBox = GetHandle();

	for(auto it = values.rbegin(); it != values.rend(); ++it) {
		auto button = static_cast<Button>(*it);
		switch(button) {
		case Button::APPLY:
			{
				AddButton(locale::get_text("apply"), button);
				break;
			}
		case Button::CANCEL:
			{
				AddButton(locale::get_text("cancel"), button);
				break;
			}
		case Button::YES:
			{
				AddButton(locale::get_text("yes"), button);
				break;
			}
		case Button::NO:
			{
				AddButton(locale::get_text("no"), button);
				break;
			}
		case Button::ACCEPT:
			{
				AddButton(locale::get_text("accept"), button);
				break;
			}
		case Button::DECLINE:
			{
				AddButton(locale::get_text("decline"), button);
				break;
			}
		case Button::OK:
			{
				AddButton(locale::get_text("ok"), button);
				break;
			}
		}
	}
}

void pragma::gui::types::WIMessageBox::Initialize()
{
	WIBase::Initialize();
	m_hBg = CreateChild<WIRect>();
	auto *pRect = m_hBg.get<WIRect>();
	pRect->SetColor(0.f, 0.f, 0.f, 0.8f);
	pRect->SetAutoAlignToParent(true);

	m_hMessage = CreateChild<WIFrame>();
	auto *pMessage = m_hMessage.get<WIFrame>();
	pMessage->SetWidth(512);
	pMessage->SetCloseButtonEnabled(false);
	auto hMessageBox = GetHandle();
	pMessage->AddCallback("SetSize", FunctionCallback<>::Create([hMessageBox]() mutable {
		if(!hMessageBox.IsValid())
			return;
		auto *pMessageBox = hMessageBox.get<WIMessageBox>();
		if(!pMessageBox->m_hMessage.IsValid())
			return;
		auto *pMessage = pMessageBox->m_hMessage.get<WIFrame>();
		auto &buttons = pMessageBox->m_buttons;
		Int32 xMargin = MARGIN_X;
		auto wMessage = pMessage->GetWidth();
		Float xOffset = CFloat(wMessage - xMargin * 2);
		if(xOffset < 0)
			xOffset = 0;
		auto numButtons = buttons.size();
		xOffset = xOffset / CFloat(numButtons - 1);
		Float x = CFloat(xMargin);
		if(numButtons == 1)
			x = wMessage * 0.5f;
		Int32 y = pMessage->GetHeight() - MARGIN_Y;
		for(auto it = buttons.begin(); it != buttons.end(); ++it) {
			auto &hButton = *it;
			if(hButton.IsValid()) {
				auto *pButton = hButton.get<WIButton>();
				auto wButton = pButton->GetWidth();
				auto xButton = CInt32(x) - CInt32(CFloat(wButton) * 0.5f);
				if(xButton < xMargin)
					xButton = xMargin;
				else {
					auto xRight = xButton + pButton->GetWidth();
					if(xRight > wMessage - xMargin)
						xButton = wMessage - xMargin - wButton;
				}
				pButton->SetPos(xButton, y);
			}
			x += xOffset;
		}
		if(pMessageBox->m_hText.IsValid()) {
			auto *pText = pMessageBox->m_hText.get<WIText>();
			pText->SetWidth(wMessage - MARGIN_X * 2);
			pText->SizeToContents();
		}
	}));
}
void pragma::gui::types::WIMessageBox::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);

	if(m_hMessage.IsValid()) {
		auto *pMessage = m_hMessage.get<WIFrame>();
		pMessage->SetPos(CInt32(CFloat(x) * 0.5f - CFloat(pMessage->GetWidth()) * 0.5f), CInt32(CFloat(y) * 0.5f - CFloat(pMessage->GetHeight()) * 0.5f));
	}
}

void pragma::gui::types::WIMessageBox::SetText(const std::string &text)
{
	if(!m_hText.IsValid())
		return;
	auto *pText = m_hText.get<WIText>();
	pText->SetText(text);
	if(!m_hMessage.IsValid())
		return;
	auto numButtons = m_buttons.size();
	auto *pMessage = m_hMessage.get<WIFrame>();
	pMessage->SetWidth(200 + CUInt32(numButtons) * 80);
	auto h = pText->GetHeight();
	pMessage->SetHeight(h + 120);
	pMessage->CenterToParent();
	pMessage->SetMinSize(pMessage->GetWidth(), pMessage->GetHeight());
	pMessage->SetMaxSize(800, 400);
}

void pragma::gui::types::WIMessageBox::SetButtonCallback(const std::function<void(WIMessageBox *, Button)> &callback) { m_buttonCallback = callback; }

pragma::gui::types::WIMessageBox::Button operator|=(const pragma::gui::types::WIMessageBox::Button &a, const pragma::gui::types::WIMessageBox::Button &b) { return static_cast<pragma::gui::types::WIMessageBox::Button>(CInt32(a) | CInt32(b)); }

pragma::gui::types::WIMessageBox::Button operator|(const pragma::gui::types::WIMessageBox::Button &a, const pragma::gui::types::WIMessageBox::Button &b) { return static_cast<pragma::gui::types::WIMessageBox::Button>(CInt32(a) | CInt32(b)); }

pragma::gui::types::WIMessageBox::Button operator&(const pragma::gui::types::WIMessageBox::Button &a, pragma::gui::types::WIMessageBox::Button &b) { return static_cast<pragma::gui::types::WIMessageBox::Button>(CInt32(a) & CInt32(b)); }
