// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.main_menu_base;
import :gui.options_list;

import :client_state;
import pragma.gui;
import pragma.string.unicode;

pragma::gui::types::WIMainMenuBase::WIMainMenuBase() : WIBase(), m_selected(-1) { AddStyleClass("main_menu"); }

void pragma::gui::types::WIMainMenuBase::OnGoBack(int button, int action, int)
{
	if(static_cast<platform::MouseButton>(button) != platform::MouseButton::Left || static_cast<platform::KeyState>(action) != platform::KeyState::Press)
		return;
	WIMainMenu *mainMenu = dynamic_cast<WIMainMenu *>(GetParent());
	if(mainMenu == nullptr)
		return;
	mainMenu->OpenMainMenu();
}

void pragma::gui::types::WIMainMenuBase::Initialize()
{
	WIBase::Initialize();
	SetSize(1'024, 768);
	m_menuElementsContainer = WGUI::GetInstance().Create<VBox>(this)->GetHandle();
	m_menuElementsContainer->AddStyleClass("main_menu_nav_container");
	ScheduleUpdate();
}
void pragma::gui::types::WIMainMenuBase::DoUpdate()
{
	WIBase::DoUpdate();
	UpdateElements();
}
pragma::util::EventReply pragma::gui::types::WIMainMenuBase::MouseCallback(platform::MouseButton, platform::KeyState state, platform::Modifier)
{
	if(state != platform::KeyState::Press)
		return util::EventReply::Handled;
	return util::EventReply::Handled;
}
pragma::util::EventReply pragma::gui::types::WIMainMenuBase::KeyboardCallback(platform::Key key, int, platform::KeyState state, platform::Modifier)
{
	if(key == platform::Key::Enter) {
		WIMainMenuElement *el = GetSelectedElement();
		if(el == nullptr)
			return util::EventReply::Handled;
		if(state == platform::KeyState::Press)
			el->Activate();
		return util::EventReply::Handled;
	}
	if(state != platform::KeyState::Press)
		return util::EventReply::Unhandled;
	if(key == platform::Key::S || key == platform::Key::Down)
		SelectNextItem();
	else if(key == platform::Key::W || key == platform::Key::Up)
		SelectPreviousItem();
	return util::EventReply::Unhandled;
}
void pragma::gui::types::WIMainMenuBase::SelectItem(int i)
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
pragma::gui::types::WIMainMenuElement *pragma::gui::types::WIMainMenuBase::GetSelectedElement()
{
	if(m_selected == -1 || m_selected >= m_elements.size())
		return nullptr;
	auto &hElement = m_elements[m_selected];
	if(!hElement.IsValid())
		return nullptr;
	return static_cast<WIMainMenuElement *>(hElement.get());
}
void pragma::gui::types::WIMainMenuBase::SelectNextItem()
{
	if(m_selected == -1) {
		SelectItem(0);
		return;
	}
	SelectItem((m_selected < m_elements.size() - 1) ? (m_selected + 1) : 0);
}
void pragma::gui::types::WIMainMenuBase::SelectPreviousItem()
{
	if(m_selected == -1) {
		SelectItem(CInt32(m_elements.size()) - 1);
		return;
	}
	SelectItem((m_selected > 0) ? (m_selected - 1) : (CInt32(m_elements.size()) - 1));
}
void pragma::gui::types::WIMainMenuBase::OnElementSelected(WIMainMenuElement *el)
{
	for(unsigned int i = 0; i < m_elements.size(); i++) {
		WIMainMenuElement *elCur = GetElement(i);
		if(elCur != nullptr) {
			if(elCur == el)
				m_selected = i;
			else
				elCur->Deselect();
		}
	}
}
void pragma::gui::types::WIMainMenuBase::UpdateElement(int i)
{
	WIMainMenuElement *el = GetElement(i);
	if(el == nullptr)
		return;
	/*if(i == 0)
		el->SetPos(0, 0);
	else {
		WIMainMenuElement *prev = GetElement(i - 1);
		if(prev != nullptr) {
			int yGap = 5;
			el->SetPos(0, prev->GetPos().y + prev->GetHeight() + yGap);
		}
	}*/
}
void pragma::gui::types::WIMainMenuBase::UpdateElements()
{
	//for(unsigned int i = 0; i < m_elements.size(); i++)
	//	UpdateElement(i);
	//if(m_menuElementsContainer.IsValid())
	//	m_menuElementsContainer->SetY(GetHeight() - m_menuElementsContainer->GetHeight() - 120);

	/*if(m_menuElementsContainer.IsValid()) {
		for(auto &hEl : m_optionLists) {
			if(hEl.IsValid() == false)
				continue;
			static_cast<WIOptionsList *>(hEl.get())->SetMaxHeight(m_menuElementsContainer->GetY() - hEl->GetY() - 38);
		}
	}*/
}
void pragma::gui::types::WIMainMenuBase::AddMenuItem(int pos, std::string name, const CallbackHandle &onActivated)
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
pragma::gui::types::WIMainMenuElement *pragma::gui::types::WIMainMenuBase::GetElement(int i)
{
	if(i >= m_elements.size())
		return nullptr;
	auto &hElement = m_elements[i];
	if(!hElement.IsValid())
		return nullptr;
	return static_cast<WIMainMenuElement *>(hElement.get());
}
void pragma::gui::types::WIMainMenuBase::RemoveMenuItem(int i)
{
	if(i >= m_elements.size())
		return;
	auto &hEl = m_elements[i];
	if(hEl.IsValid())
		hEl->Remove();
	m_elements.erase(m_elements.begin() + i);
	UpdateElements();
}
void pragma::gui::types::WIMainMenuBase::AddMenuItem(std::string name, const CallbackHandle &onActivated) { AddMenuItem(CInt32(m_elements.size()), name, onActivated); }
pragma::gui::types::WIOptionsList *pragma::gui::types::WIMainMenuBase::InitializeOptionsList()
{
	m_hControlSettings = CreateChild<WIOptionsList>();
	m_hControlSettings->SetName("options");
	return static_cast<WIOptionsList *>(m_hControlSettings.get());
}
void pragma::gui::types::WIMainMenuBase::InitializeOptionsList(WIOptionsList *pList)
{
	pList->SizeToContents();
	pList->ScheduleUpdate();
	m_optionLists.push_back(pList->GetHandle());
}

////////////////////////////

pragma::gui::types::WIMainMenuElement::WIMainMenuElement() : WIBase(), onActivated(), onSelected(), onDeselected(), m_bSelected(false)
{
	RegisterCallback<void>("Select");
	RegisterCallback<void>("Deselect");
}

pragma::gui::types::WIMainMenuElement::~WIMainMenuElement() {}

void pragma::gui::types::WIMainMenuElement::Select()
{
	if(m_bSelected == true)
		return;
	m_bSelected = true;
	SetInputState(InputState::Hover);
	CallCallbacks<void>("Select");
	if(onSelected == nullptr)
		return;
	onSelected(this);
}

void pragma::gui::types::WIMainMenuElement::Deselect()
{
	if(m_bSelected == false)
		return;
	m_bSelected = false;
	SetInputState(InputState::None);
	CallCallbacks<void>("Deselect");
	if(onDeselected == nullptr)
		return;
	onDeselected(this);
}

void pragma::gui::types::WIMainMenuElement::Initialize()
{
	WIBase::Initialize();

	SetSize(100, 20);
	AddStyleClass("main_menu_nav_item");
	WIText *pText = WGUI::GetInstance().Create<WIText>(this);
	if(pText != nullptr)
		m_hText = pText->GetHandle();
	SetMouseInputEnabled(true);
}

void pragma::gui::types::WIMainMenuElement::Activate()
{
	if(onActivated == nullptr)
		return;
	onActivated(this);
}

pragma::util::EventReply pragma::gui::types::WIMainMenuElement::MouseCallback(platform::MouseButton button, platform::KeyState state, platform::Modifier mods)
{
	WIBase::MouseCallback(button, state, mods);
	if(state != platform::KeyState::Press)
		return util::EventReply::Handled;
	Activate();
	return util::EventReply::Handled;
}

void pragma::gui::types::WIMainMenuElement::OnCursorEntered() { Select(); }
void pragma::gui::types::WIMainMenuElement::OnCursorExited() { Deselect(); }

void pragma::gui::types::WIMainMenuElement::SetText(std::string &text)
{
	if(m_hText.IsValid()) {
		WIText *pText = static_cast<WIText *>(m_hText.get());
		pText->SetText(text);
		pText->SizeToContents();
	}
}
Vector4 pragma::gui::types::WIMainMenuElement::GetBackgroundColor()
{
	/*if(!m_hBackground.IsValid())
		return Vector4(1, 1, 1, 1);
	return m_hBackground->GetColor().ToVector4();*/
	return Vector4{};
}
void pragma::gui::types::WIMainMenuElement::SetBackgroundColor(float r, float g, float b, float a)
{
	/*if(!m_hBackground.IsValid())
		return;
	m_hBackground->SetColor(r, g, b, a);*/
}
