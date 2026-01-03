// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.choice_list;

import pragma.gui;
import pragma.string.unicode;

pragma::gui::types::WIChoiceList::WIChoiceList() : WIBase(), m_selected(UInt(-1)) { RegisterCallback<void, uint32_t, std::reference_wrapper<std::string>>("OnSelect"); }

pragma::gui::types::WIChoiceList::~WIChoiceList() {}

void pragma::gui::types::WIChoiceList::Initialize()
{
	WIBase::Initialize();
	m_text = CreateChild<WIText>();
	m_text->SetAutoCenterToParent(true);
	m_buttonPrev = CreateChild<WIButton>();
	WIHandle hChoiceList = GetHandle();
	auto *buttonPrev = static_cast<WIButton *>(m_buttonPrev.get());
	buttonPrev->SetText("<");
	buttonPrev->AddCallback("OnPressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([hChoiceList](util::EventReply *reply) mutable -> CallbackReturnType {
		*reply = util::EventReply::Handled;
		if(!hChoiceList.IsValid())
			return CallbackReturnType::HasReturnValue;
		static_cast<WIChoiceList *>(hChoiceList.get())->SelectPrevious();
		return CallbackReturnType::HasReturnValue;
	}));

	m_buttonNext = CreateChild<WIButton>();
	auto *buttonNext = static_cast<WIButton *>(m_buttonNext.get());
	buttonNext->SetText(">");
	buttonNext->AddCallback("OnPressed", FunctionCallback<util::EventReply>::CreateWithOptionalReturn([hChoiceList](util::EventReply *reply) mutable -> CallbackReturnType {
		*reply = util::EventReply::Handled;
		if(!hChoiceList.IsValid())
			return CallbackReturnType::HasReturnValue;
		static_cast<WIChoiceList *>(hChoiceList.get())->SelectNext();
		return CallbackReturnType::HasReturnValue;
	}));
}

void pragma::gui::types::WIChoiceList::UpdateButtons()
{
	if(m_buttonPrev.IsValid())
		m_buttonPrev->SetVisible((m_selected > 0) ? true : false);
	if(m_buttonNext.IsValid())
		m_buttonNext->SetVisible((m_selected < (m_choices.size() - 1)) ? true : false);
}

void pragma::gui::types::WIChoiceList::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	if(m_buttonPrev.IsValid()) {
		auto *buttonPrev = static_cast<WIButton *>(m_buttonPrev.get());
		buttonPrev->SetSize(y, y);
	}
	if(m_buttonNext.IsValid()) {
		auto *buttonNext = static_cast<WIButton *>(m_buttonNext.get());
		buttonNext->SetSize(y, y);
		buttonNext->SetX(x - buttonNext->GetWidth());
	}
}

void pragma::gui::types::WIChoiceList::SetChoices(const std::vector<std::string> &choices)
{
	UInt idx = 0;
	for(auto it = choices.begin(); it != choices.end(); ++it) {
		AddChoice(*it, std::to_string(idx));
		idx++;
	}
}
void pragma::gui::types::WIChoiceList::SetChoices(const std::vector<std::pair<std::string, std::string>> &choices)
{
	for(auto &pair : choices)
		AddChoice(pair.second, pair.first);
}
void pragma::gui::types::WIChoiceList::AddChoice(const std::string &choice, const std::string &val)
{
	m_choices.push_back(Choice(choice, val));
	if(m_choices.size() == 1)
		SelectChoice(0);
	UpdateButtons();
}
void pragma::gui::types::WIChoiceList::AddChoice(const std::string &choice) { AddChoice(choice, std::to_string(m_choices.size())); }
uint32_t pragma::gui::types::WIChoiceList::GetChoiceCount() const { return static_cast<uint32_t>(m_choices.size()); }

void pragma::gui::types::WIChoiceList::SelectChoice(UInt ichoice)
{
	if(ichoice >= m_choices.size())
		return;
	m_selected = ichoice;
	if(!m_text.IsValid())
		return;
	auto *pText = static_cast<WIText *>(m_text.get());
	auto &choice = m_choices[ichoice];
	pText->SetText(choice.choice);
	pText->SizeToContents();
	pText->SetX(CInt32(CFloat(GetWidth()) * 0.5f - CFloat(pText->GetWidth()) * 0.5f));
	UpdateButtons();
	CallCallbacks<void, uint32_t, std::reference_wrapper<std::string>>("OnSelect", ichoice, std::ref(choice.value));
}

void pragma::gui::types::WIChoiceList::SelectChoice(const std::string &choiceName)
{
	UInt i = 0;
	for(auto it = m_choices.begin(); it != m_choices.end(); ++it, ++i) {
		auto &choice = *it;
		if(choice.value == choiceName) {
			SelectChoice(i);
			break;
		}
	}
}

void pragma::gui::types::WIChoiceList::SelectNext()
{
	if(m_selected >= m_choices.size() - 1)
		return;
	SelectChoice(m_selected + 1);
}
void pragma::gui::types::WIChoiceList::SelectPrevious()
{
	if(m_selected == 0)
		return;
	SelectChoice(m_selected - 1);
}
const pragma::gui::types::WIChoiceList::Choice *pragma::gui::types::WIChoiceList::GetSelectedChoice()
{
	if(m_selected >= m_choices.size())
		return nullptr;
	return &m_choices.at(m_selected);
}
