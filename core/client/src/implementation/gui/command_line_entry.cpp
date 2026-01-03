// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.command_line_entry;

import pragma.gui;
import pragma.string.unicode;

pragma::gui::types::WICommandLineEntry::WICommandLineEntry() : WITextEntry() {}

void pragma::gui::types::WICommandLineEntry::Initialize()
{
	WITextEntry::Initialize();
	SetAutoCompleteEntryLimit(10);
	if(m_hBase.IsValid()) {
		auto hThis = GetHandle();
		m_hBase->AddCallback("OnKeyEvent",
		  FunctionCallback<util::EventReply, platform::Key, int, platform::KeyState, platform::Modifier>::CreateWithOptionalReturn([hThis, this](util::EventReply *reply, platform::Key key, int scanCode, platform::KeyState state, platform::Modifier mods) -> CallbackReturnType {
			  if((state != platform::KeyState::Press && state != platform::KeyState::Repeat) || hThis.IsValid() == false)
				  return CallbackReturnType::NoReturnValue;
			  const auto fApplyText = [this](WIMenuItem *pItem) {
				  if(pItem == nullptr)
					  return;
				  auto *pTextEl = pItem->GetTextElement();
				  if(pTextEl == nullptr)
					  return;
				  m_bSkipAutoComplete = true;
				  auto &text = pTextEl->GetText();
				  auto insertText = text.cpp_str() + ' '; // Add a space to the end of the command so the user can add arguments immediately and can skip typing the space themselves
				  SetText(insertText);
				  if(text.empty() == false)
					  SetCaretPos(insertText.length());
				  m_bSkipAutoComplete = false;
			  };
			  switch(key) {
			  case platform::Key::Up:
				  {
					  auto *pContextMenu = static_cast<WIContextMenu *>(m_hAutoCompleteList.get());
					  if(pContextMenu != nullptr) {
						  auto optIdx = pContextMenu->GetSelectedItemIndex();
						  auto count = pContextMenu->GetItemCount();
						  if(count > 0u) {
							  auto idx = optIdx.has_value() ? ((*optIdx > 0u) ? (*optIdx - 1u) : (count - 1u)) : (count - 1u);
							  fApplyText(pContextMenu->SelectItem(idx));
						  }
					  }
					  break;
				  }
			  case platform::Key::Down:
				  {
					  auto *pContextMenu = static_cast<WIContextMenu *>(m_hAutoCompleteList.get());
					  if(pContextMenu != nullptr) {
						  auto optIdx = pContextMenu->GetSelectedItemIndex();
						  auto count = pContextMenu->GetItemCount();
						  if(count > 0u) {
							  auto idx = optIdx.has_value() ? ((*optIdx + 1u) % count) : 0u;
							  fApplyText(pContextMenu->SelectItem(idx));
						  }
					  }
					  break;
				  }
			  default:
				  return CallbackReturnType::NoReturnValue;
			  }
			  *reply = util::EventReply::Handled;
			  return CallbackReturnType::HasReturnValue;
		  }));
	}
}

void pragma::gui::types::WICommandLineEntry::OnTextEntered()
{
	WITextEntry::OnTextEntered();
	auto text = GetText();
	if(text.empty() == false)
		AddCommandHistoryEntry(text.cpp_str());
	if(m_hAutoCompleteList.IsValid())
		m_hAutoCompleteList->Remove();
	SetText("");
	RequestFocus();
}

void pragma::gui::types::WICommandLineEntry::OnFocusGained()
{
	WITextEntry::OnFocusGained();
	InitializeAutoCompleteList();
}

void pragma::gui::types::WICommandLineEntry::OnFocusKilled()
{
	WITextEntry::OnFocusKilled();
	if(m_hAutoCompleteList.IsValid())
		m_hAutoCompleteList->Remove();
}

void pragma::gui::types::WICommandLineEntry::InitializeAutoCompleteList()
{
	auto *pText = GetTextElement();
	if(pText == nullptr || m_bAutocompleteEnabled == false)
		return;
	std::vector<std::string> options {};
	auto &text = pText->GetText();
	if(text.empty()) {
		auto numCmds = math::min(static_cast<int32_t>(m_commandHistoryCount), static_cast<int32_t>(GetAutoCompleteEntryLimit()));
		options.resize(numCmds);
		for(auto i = 0; i < numCmds; ++i) {
			auto idx = static_cast<int32_t>(m_nextCommandHistoryInsertPos) - 1 - i;
			if(idx < 0)
				idx += m_commandHistory.size();
			options[numCmds - i - 1] = m_commandHistory.at(idx);
		}
	}
	else
		FindAutocompleteOptions(text.cpp_str(), options);
	if(options.empty())
		return;
	auto pos = GetAbsolutePos();
	auto size = GetSize();
	if(m_hAutoCompleteList.IsValid() == false) {
		auto *pContextMenu = WGUI::GetInstance().Create<WIContextMenu>();
		pContextMenu->SetParentAndUpdateWindow(GetRootElement());
		pContextMenu->SetZPos(100'000);
		m_hAutoCompleteList = pContextMenu->GetHandle();
		RemoveOnRemoval(pContextMenu);
	}
	auto *pContextMenu = static_cast<WIContextMenu *>(m_hAutoCompleteList.get());
	pContextMenu->SetPos(pos.x, pos.y + size.y);
	pContextMenu->ClearItems();
	auto hThis = GetHandle();
	for(auto &option : options) {
		auto *pItem = pContextMenu->AddItem(option, [hThis, this, option](WIMenuItem &item) -> bool {
			if(hThis.IsValid()) {
				SetText(option);
				if(hThis.IsValid())
					SetCaretPos(option.length());
			}
			return true;
		});
		/*pItem->AddCallback("OnSelected",FunctionCallback<void>::Create([this,hThis,&pItem,option]() {
			if(hThis.IsValid() == false)
				return;
			m_bSkipAutoComplete = true;
			SetText(option);
			m_bSkipAutoComplete = false;
		}));*/
	}
	pContextMenu->Update();
	auto *p = pContextMenu->GetParent();
	if(p && p->IsInBounds(pContextMenu->GetX(), pContextMenu->GetY(), pContextMenu->GetWidth(), pContextMenu->GetHeight()) != math::intersection::Intersect::Inside)
		pContextMenu->SetY(pos.y - pContextMenu->GetHeight());
	// pContextMenu->SetWidth(size.x);
}
void pragma::gui::types::WICommandLineEntry::SetAutocompleteEnabled(bool enabled)
{
	m_bAutocompleteEnabled = enabled;
	if(enabled) {
		if(HasFocus())
			InitializeAutoCompleteList();
	}
	else if(m_hAutoCompleteList.IsValid())
		m_hAutoCompleteList->Remove();
}
bool pragma::gui::types::WICommandLineEntry::IsAutocompleteEnabled() const { return m_bAutocompleteEnabled; }
void pragma::gui::types::WICommandLineEntry::OnTextChanged(const string::Utf8String &text, bool changedByUser)
{
	WITextEntry::OnTextChanged(text, changedByUser);
	if(m_bSkipAutoComplete)
		return;
	InitializeAutoCompleteList();
}

void pragma::gui::types::WICommandLineEntry::SetAutoCompleteEntryLimit(uint32_t limit) { m_commandHistory.resize(limit); }
uint32_t pragma::gui::types::WICommandLineEntry::GetAutoCompleteEntryLimit() const { return m_commandHistory.size(); }
void pragma::gui::types::WICommandLineEntry::FindAutocompleteOptions(const std::string &cmd, std::vector<std::string> &args) const { m_fAutoCompleteHandler(cmd, args); }
void pragma::gui::types::WICommandLineEntry::SetAutocompleteHandler(const std::function<void(const std::string &, std::vector<std::string> &)> &fAutoCompleteHandler) { m_fAutoCompleteHandler = fAutoCompleteHandler; }
void pragma::gui::types::WICommandLineEntry::ClearCommandHistory()
{
	m_commandHistory.clear();
	m_commandHistoryCount = 0u;
	m_nextCommandHistoryInsertPos = 0u;
}
void pragma::gui::types::WICommandLineEntry::AddCommandHistoryEntry(const std::string_view &entry)
{
	m_commandHistory.at(m_nextCommandHistoryInsertPos) = std::string {entry};
	m_nextCommandHistoryInsertPos = (m_nextCommandHistoryInsertPos + 1u) % m_commandHistory.size();
	m_commandHistoryCount = math::min(m_commandHistoryCount + 1u, static_cast<uint32_t>(m_commandHistory.size()));
}
