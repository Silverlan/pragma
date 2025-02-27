/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/gui/wichoicelist.h"
#include <wgui/types/witext.h>
#include <wgui/types/wibutton.h>

import pragma.locale;
import pragma.string.unicode;

LINK_WGUI_TO_CLASS(WIChoiceList, WIChoiceList);

WIChoiceList::WIChoiceList() : WIBase(), m_selected(UInt(-1)) { RegisterCallback<void, uint32_t, std::reference_wrapper<std::string>>("OnSelect"); }

WIChoiceList::~WIChoiceList() {}

void WIChoiceList::Initialize()
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

void WIChoiceList::UpdateButtons()
{
	if(m_buttonPrev.IsValid())
		m_buttonPrev->SetVisible((m_selected > 0) ? true : false);
	if(m_buttonNext.IsValid())
		m_buttonNext->SetVisible((m_selected < (m_choices.size() - 1)) ? true : false);
}

void WIChoiceList::SetSize(int x, int y)
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

void WIChoiceList::SetChoices(const std::vector<std::string> &choices)
{
	UInt idx = 0;
	for(auto it = choices.begin(); it != choices.end(); ++it) {
		AddChoice(*it, std::to_string(idx));
		idx++;
	}
}
void WIChoiceList::SetChoices(const std::vector<std::pair<std::string, std::string>> &choices)
{
	for(auto &pair : choices)
		AddChoice(pair.second, pair.first);
}
void WIChoiceList::AddChoice(const std::string &choice, const std::string &val)
{
	m_choices.push_back(Choice(choice, val));
	if(m_choices.size() == 1)
		SelectChoice(0);
	UpdateButtons();
}
void WIChoiceList::AddChoice(const std::string &choice) { AddChoice(choice, std::to_string(m_choices.size())); }
uint32_t WIChoiceList::GetChoiceCount() const { return static_cast<uint32_t>(m_choices.size()); }

void WIChoiceList::SelectChoice(UInt ichoice)
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

void WIChoiceList::SelectChoice(const std::string &choiceName)
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

void WIChoiceList::SelectNext()
{
	if(m_selected >= m_choices.size() - 1)
		return;
	SelectChoice(m_selected + 1);
}
void WIChoiceList::SelectPrevious()
{
	if(m_selected == 0)
		return;
	SelectChoice(m_selected - 1);
}
const WIChoiceList::Choice *WIChoiceList::GetSelectedChoice()
{
	if(m_selected >= m_choices.size())
		return nullptr;
	return &m_choices.at(m_selected);
}
