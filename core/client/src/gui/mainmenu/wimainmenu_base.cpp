// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/gui/mainmenu/wimainmenu_base.h"
#include "pragma/gui/mainmenu/wimainmenu.h"
#include <pragma/string/format.h>
#include <wgui/types/wibutton.h>
#include "pragma/gui/wioptionslist.h"
#include <wgui/types/witext.h>
#include <wgui/types/wirect.h>

import pragma.locale;
import pragma.string.unicode;

LINK_WGUI_TO_CLASS(WIMainMenuElement, WIMainMenuElement);

extern ClientState *client;
WIMainMenuBase::WIMainMenuBase() : WIBase(), m_selected(-1) { AddStyleClass("main_menu"); }

void WIMainMenuBase::OnGoBack(int button, int action, int)
{
	if(static_cast<pragma::platform::MouseButton>(button) != pragma::platform::MouseButton::Left || static_cast<pragma::platform::KeyState>(action) != pragma::platform::KeyState::Press)
		return;
	WIMainMenu *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
	if(mainMenu == nullptr)
		return;
	mainMenu->OpenMainMenu();
}

void WIMainMenuBase::Initialize()
{
	WIBase::Initialize();
	SetSize(1'024, 768);
	m_menuElementsContainer = WGUI::GetInstance().Create<WIBase>(this)->GetHandle();
	m_menuElementsContainer->SetAutoSizeToContents(true);
	m_menuElementsContainer->SetX(168);
	ScheduleUpdate();
}
void WIMainMenuBase::DoUpdate()
{
	WIBase::DoUpdate();
	UpdateElements();
}
util::EventReply WIMainMenuBase::MouseCallback(pragma::platform::MouseButton, pragma::platform::KeyState state, pragma::platform::Modifier)
{
	if(state != pragma::platform::KeyState::Press)
		return util::EventReply::Handled;
	return util::EventReply::Handled;
}
util::EventReply WIMainMenuBase::KeyboardCallback(pragma::platform::Key key, int, pragma::platform::KeyState state, pragma::platform::Modifier)
{
	if(key == pragma::platform::Key::Enter) {
		WIMainMenuElement *el = GetSelectedElement();
		if(el == NULL)
			return util::EventReply::Handled;
		if(state == pragma::platform::KeyState::Press)
			el->Activate();
		return util::EventReply::Handled;
	}
	if(state != pragma::platform::KeyState::Press)
		return util::EventReply::Unhandled;
	if(key == pragma::platform::Key::S || key == pragma::platform::Key::Down)
		SelectNextItem();
	else if(key == pragma::platform::Key::W || key == pragma::platform::Key::Up)
		SelectPreviousItem();
	return util::EventReply::Unhandled;
}
void WIMainMenuBase::SelectItem(int i)
{
	if(m_selected >= m_elements.size())
		m_selected = -1;
	if(m_selected == -1) {
		if(i >= m_elements.size())
			return;
		if(m_elements[i].IsValid() == false)
			return;
		m_selected = i;
		static_cast<WIMainMenuElement *>(m_elements[i].get())->Select();
		return;
	}
	if(m_elements[m_selected].IsValid())
		static_cast<WIMainMenuElement *>(m_elements[m_selected].get())->Deselect();
	if(i >= m_elements.size()) {
		m_selected = -1;
		return;
	}
	m_selected = i;
	static_cast<WIMainMenuElement *>(m_elements[i].get())->Select();
}
WIMainMenuElement *WIMainMenuBase::GetSelectedElement()
{
	if(m_selected == -1 || m_selected >= m_elements.size())
		return NULL;
	auto &hElement = m_elements[m_selected];
	if(!hElement.IsValid())
		return NULL;
	return static_cast<WIMainMenuElement *>(hElement.get());
}
void WIMainMenuBase::SelectNextItem()
{
	if(m_selected == -1) {
		SelectItem(0);
		return;
	}
	SelectItem((m_selected < m_elements.size() - 1) ? (m_selected + 1) : 0);
}
void WIMainMenuBase::SelectPreviousItem()
{
	if(m_selected == -1) {
		SelectItem(CInt32(m_elements.size()) - 1);
		return;
	}
	SelectItem((m_selected > 0) ? (m_selected - 1) : (CInt32(m_elements.size()) - 1));
}
void WIMainMenuBase::OnElementSelected(WIMainMenuElement *el)
{
	for(unsigned int i = 0; i < m_elements.size(); i++) {
		WIMainMenuElement *elCur = GetElement(i);
		if(elCur != NULL) {
			if(elCur == el)
				m_selected = i;
			else
				elCur->Deselect();
		}
	}
}
void WIMainMenuBase::UpdateElement(int i)
{
	WIMainMenuElement *el = GetElement(i);
	if(el == NULL)
		return;
	if(i == 0)
		el->SetPos(0, 0);
	else {
		WIMainMenuElement *prev = GetElement(i - 1);
		if(prev != NULL) {
			int yGap = 5;
			el->SetPos(0, prev->GetPos().y + prev->GetHeight() + yGap);
		}
	}
}
void WIMainMenuBase::UpdateElements()
{
	for(unsigned int i = 0; i < m_elements.size(); i++)
		UpdateElement(i);
	if(m_menuElementsContainer.IsValid())
		m_menuElementsContainer->SetY(GetHeight() - m_menuElementsContainer->GetHeight() - 120);

	if(m_menuElementsContainer.IsValid()) {
		for(auto &hEl : m_optionLists) {
			if(hEl.IsValid() == false)
				continue;
			static_cast<WIOptionsList *>(hEl.get())->SetMaxHeight(m_menuElementsContainer->GetY() - hEl->GetY() - 38);
		}
	}
}
void WIMainMenuBase::AddMenuItem(int pos, std::string name, const CallbackHandle &onActivated)
{
	auto *el = WGUI::GetInstance().Create<WIMainMenuElement>(m_menuElementsContainer.get());
	el->SetText(name);
	el->onActivated = onActivated;
	el->onSelected = FunctionCallback<void, WIMainMenuElement *>::Create([](WIMainMenuElement *el) {
		auto *parent = dynamic_cast<WIMainMenuBase *>(el->GetParent());
		if(parent == nullptr)
			return;
		parent->OnElementSelected(el);
	});
	el->AddStyleClass("main_menu_text");
	if(pos >= m_elements.size()) {
		m_elements.push_back(el->GetHandle());
		UpdateElement(CInt32(m_elements.size()) - 1);
	}
	else {
		m_elements.insert(m_elements.begin() + pos, el->GetHandle());
		UpdateElements();
	}
}
WIMainMenuElement *WIMainMenuBase::GetElement(int i)
{
	if(i >= m_elements.size())
		return NULL;
	auto &hElement = m_elements[i];
	if(!hElement.IsValid())
		return NULL;
	return static_cast<WIMainMenuElement *>(hElement.get());
}
void WIMainMenuBase::RemoveMenuItem(int i)
{
	if(i >= m_elements.size())
		return;
	auto &hEl = m_elements[i];
	if(hEl.IsValid())
		hEl->Remove();
	m_elements.erase(m_elements.begin() + i);
	UpdateElements();
}
void WIMainMenuBase::AddMenuItem(std::string name, const CallbackHandle &onActivated) { AddMenuItem(CInt32(m_elements.size()), name, onActivated); }
WIOptionsList *WIMainMenuBase::InitializeOptionsList()
{
	m_hControlSettings = CreateChild<WIOptionsList>();
	m_hControlSettings->SetName("options");
	return static_cast<WIOptionsList *>(m_hControlSettings.get());
}
void WIMainMenuBase::InitializeOptionsList(WIOptionsList *pList)
{
	pList->SetPos(192, 200);
	pList->SizeToContents();
	pList->SetWidth(700);
	pList->ScheduleUpdate();
	m_optionLists.push_back(pList->GetHandle());
}

////////////////////////////

WIMainMenuElement::WIMainMenuElement() : WIBase(), onActivated(), onSelected(), onDeselected(), m_bSelected(false)
{
	RegisterCallback<void>("Select");
	RegisterCallback<void>("Deselect");
}

WIMainMenuElement::~WIMainMenuElement() {}

void WIMainMenuElement::Select()
{
	if(m_bSelected == true)
		return;
	m_bSelected = true;

	if(m_hBackground.IsValid())
		m_hBackground->SetVisible(true);
	CallCallbacks<void>("Select");
	if(onSelected == nullptr)
		return;
	onSelected(this);
}

void WIMainMenuElement::Deselect()
{
	if(m_bSelected == false)
		return;
	m_bSelected = false;
	if(m_hBackground.IsValid())
		m_hBackground->SetVisible(false);
	CallCallbacks<void>("Deselect");
	if(onDeselected == nullptr)
		return;
	onDeselected(this);
}

void WIMainMenuElement::Initialize()
{
	WIBase::Initialize();

	SetSize(350, 46);
	auto *pBackground = WGUI::GetInstance().Create<WIRect>(this);
	if(pBackground) {
		m_hBackground = pBackground->GetHandle();
		pBackground->SetColor(Color {76, 76, 76});
		pBackground->SetVisible(false);
		pBackground->SetSize(GetSize());
		pBackground->SetAnchor(0.f, 0.f, 1.f, 1.f);

		auto *pPrefix = WGUI::GetInstance().Create<WIRect>(m_hBackground.get());
		pPrefix->SetColor(Color {255, 210, 0});
		pPrefix->SetHeight(pBackground->GetHeight());
		pPrefix->SetWidth(3);
		pPrefix->SetAnchor(0.f, 0.f, 0.f, 1.f);
	}
	WIText *pText = WGUI::GetInstance().Create<WIText>(this);
	if(pText != NULL) {
		m_hText = pText->GetHandle();
		m_hText->SetX(21);
		/*pText->SetFont("MainMenu_Regular");
		pText->SetColor(MENU_ITEM_COLOR);
		
		pText->EnableShadow(true);
		pText->SetShadowColor(MENU_ITEM_SHADOW_COLOR);
		pText->SetShadowOffset(MENU_ITEM_SHADOW_OFFSET);*/
	}
	SetMouseInputEnabled(true);
}

void WIMainMenuElement::Activate()
{
	if(onActivated == nullptr)
		return;
	onActivated(this);
}

util::EventReply WIMainMenuElement::MouseCallback(pragma::platform::MouseButton button, pragma::platform::KeyState state, pragma::platform::Modifier mods)
{
	WIBase::MouseCallback(button, state, mods);
	if(state != pragma::platform::KeyState::Press)
		return util::EventReply::Handled;
	Activate();
	return util::EventReply::Handled;
}

void WIMainMenuElement::OnCursorEntered() { Select(); }
void WIMainMenuElement::OnCursorExited() { Deselect(); }

void WIMainMenuElement::SetText(std::string &text)
{
	if(m_hText.IsValid()) {
		pragma::string::Utf8String upperText {text};
		upperText.toUpper();
		WIText *pText = static_cast<WIText *>(m_hText.get());
		pText->SetText(upperText);
		pText->SizeToContents();
	}
}
void WIMainMenuElement::SetSize(int x, int y) { WIBase::SetSize(x, y); }
Vector4 WIMainMenuElement::GetBackgroundColor()
{
	if(!m_hBackground.IsValid())
		return Vector4(1, 1, 1, 1);
	return m_hBackground->GetColor().ToVector4();
}
void WIMainMenuElement::SetBackgroundColor(float r, float g, float b, float a)
{
	if(!m_hBackground.IsValid())
		return;
	m_hBackground->SetColor(r, g, b, a);
}
