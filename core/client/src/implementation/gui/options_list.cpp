// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :gui.options_list;
import :gui.scroll_container;

import :client_state;
import :engine;
import pragma.gui;
import pragma.string.unicode;

pragma::gui::types::WIOptionsList::WIOptionsList() : WIBase() {}

pragma::gui::types::WIOptionsList::~WIOptionsList() {}

void pragma::gui::types::WIOptionsList::Initialize()
{
	WIBase::Initialize();
	m_hTable = CreateChild<WITable>();
	auto *pTable = m_hTable.get<WITable>();
	pTable->SetAnchor(0.f, 0.f, 1.f, 1.f);
	pTable->SetRowHeight(32);
	pTable->SetScrollable(true);
	//pTable->SetAnchor(0.f,0.f,1.f,1.f);
	auto *pRow = pTable->AddHeaderRow();
	m_hHeaderRow = pRow->GetHandle();
}

pragma::gui::types::WITableRow *pragma::gui::types::WIOptionsList::AddHeaderRow()
{
	if(!m_hTable.IsValid())
		return nullptr;
	auto *pTable = m_hTable.get<WITable>();
	auto *pRow = pTable->AddHeaderRow();
	return pRow;
}

void pragma::gui::types::WIOptionsList::SetUpdateConVar(const std::string &cvar, const std::string &value) { m_updateCvars[cvar] = value; }

void pragma::gui::types::WIOptionsList::SetTitle(const std::string &title)
{
	if(!m_hHeaderRow.IsValid())
		return;
	auto *pRow = m_hHeaderRow.get<WITableRow>();
	pRow->SetValue(0, title);
}

void pragma::gui::types::WIOptionsList::SetSize(int x, int y)
{
	WIBase::SetSize(x, y);
	if(m_hTable.IsValid())
		m_hTable->SetWidth(x);
	if(math::is_flag_set(m_stateFlags, StateFlags::IsBeingUpdated) == false)
		ScheduleUpdate();
}

void pragma::gui::types::WIOptionsList::DoUpdate()
{
	WIBase::DoUpdate();
	if(m_hTable.IsValid())
		m_hTable->Update();
	SizeToContents();
}

void pragma::gui::types::WIOptionsList::SetMaxHeight(uint32_t h)
{
	m_maxHeight = h;
	if(GetHeight() > h)
		SetHeight(h);
}

void pragma::gui::types::WIOptionsList::SizeToContents(bool x, bool y)
{
	if(m_hTable.IsValid()) {
		auto *pTable = m_hTable.get<WITable>();
		pTable->SizeToContents();
		auto h = pTable->GetHeight();
		if(h > m_maxHeight)
			h = m_maxHeight;
		pTable->SetSize(GetWidth(), h);
		SetHeight(h);
	}
}

pragma::gui::types::WITableRow *pragma::gui::types::WIOptionsList::AddRow(const std::optional<std::string> &identifier)
{
	if(!m_hTable.IsValid())
		return nullptr;
	auto *row = m_hTable.get<WITable>()->AddRow();
	if(row && identifier.has_value() && !identifier->empty())
		m_rows[*identifier] = row->GetHandle();
	return row;
}

pragma::gui::types::WITableRow *pragma::gui::types::WIOptionsList::GetRow(const std::string &identifier) const
{
	auto it = m_rows.find(identifier);
	return (it != m_rows.end()) ? const_cast<WITableRow *>(dynamic_cast<const WITableRow *>(it->second.get())) : nullptr;
}

pragma::gui::types::WICheckbox *pragma::gui::types::WIOptionsList::AddToggleChoice(const std::string &name, const std::string &cvarName, const std::function<std::string(bool)> &translator, const std::function<bool(std::string)> &translator2)
{
	auto *row = AddRow(cvarName);
	if(row == nullptr)
		return nullptr;
	row->SetValue(0, name);
	auto hCheckbox = CreateChild<WICheckbox>();
	auto *pCheckbox = hCheckbox.get<WICheckbox>();
	if((translator2 == nullptr) ? get_cengine()->GetConVarBool(cvarName) : translator2(get_cengine()->GetConVarString(cvarName)))
		pCheckbox->SetChecked(true);
	auto hOptions = GetHandle();
	pCheckbox->AddCallback("OnChange", FunctionCallback<void, bool>::Create([hOptions, cvarName, translator](bool bChecked) mutable {
		if(!hOptions.IsValid())
			return;
		hOptions.get<WIOptionsList>()->m_updateCvars[cvarName] = (translator == nullptr) ? std::to_string(bChecked) : translator(bChecked);
	}));
	pCheckbox->SetAutoCenterToParent(true);
	row->InsertElement(1, hCheckbox);
	return pCheckbox;
}

pragma::gui::types::WICheckbox *pragma::gui::types::WIOptionsList::AddToggleChoice(const std::string &name, const std::string &cvarName) { return AddToggleChoice(name, cvarName, nullptr); }

template<class T>
pragma::gui::types::WIChoiceList *pragma::gui::types::WIOptionsList::AddChoiceList(const std::string &name, T list, const std::string &cvarName, const std::function<void(WIChoiceList *)> &initializer, const std::optional<std::string> &optRowIdent)
{
	auto *row = AddRow(optRowIdent.has_value() ? *optRowIdent : cvarName);
	if(row == nullptr)
		return nullptr;
	auto hChoiceList = CreateChild<WIChoiceList>();
	auto *pChoiceList = hChoiceList.get<WIChoiceList>();
	auto sz = GetSize();
	pChoiceList->SetSize(sz.x, sz.y);
	pChoiceList->SetAnchor(0.f, 0.f, 1.f, 1.f);
	pChoiceList->SetChoices(list);
	row->SetValue(0, name);
	if(initializer != nullptr)
		initializer(pChoiceList);
	auto hOptions = GetHandle();
	if(cvarName.empty() == false) {
		pChoiceList->SelectChoice(get_client_state()->GetConVarString(cvarName));
		pChoiceList->AddCallback("OnSelect", FunctionCallback<void, uint32_t, std::reference_wrapper<std::string>>::Create([hOptions, cvarName](uint32_t, std::reference_wrapper<std::string> value) mutable {
			if(!hOptions.IsValid())
				return;
			hOptions.get<WIOptionsList>()->m_updateCvars[cvarName] = value;
		}));
	}
	row->InsertElement(1, pChoiceList);
	return pChoiceList;
}
pragma::gui::types::WIChoiceList *pragma::gui::types::WIOptionsList::AddChoiceList(const std::string &name, const std::vector<std::pair<std::string, std::string>> &list, const std::string &cvarName, const std::optional<std::string> &optRowIdent)
{
	return AddChoiceList<const std::vector<std::pair<std::string, std::string>> &>(name, list, cvarName, nullptr, optRowIdent);
}
pragma::gui::types::WIChoiceList *pragma::gui::types::WIOptionsList::AddChoiceList(const std::string &name, const std::vector<std::string> &list, const std::string &cvarName) { return AddChoiceList<const std::vector<std::string> &>(name, list, cvarName, nullptr); }
pragma::gui::types::WIChoiceList *pragma::gui::types::WIOptionsList::AddChoiceList(const std::string &name, const std::function<void(WIChoiceList *)> &initializer, const std::string &cvarName)
{
	return AddChoiceList<const std::vector<std::string> &>(name, std::vector<std::string>({}), cvarName, initializer);
}
pragma::gui::types::WIChoiceList *pragma::gui::types::WIOptionsList::AddChoiceList(const std::string &name, const std::string &cvarName) { return AddChoiceList(name, nullptr, cvarName); }

template<class T>
pragma::gui::types::WIDropDownMenu *pragma::gui::types::WIOptionsList::AddDropDownMenu(const std::string &name, T list, const std::string &cvarName, const std::function<void(WIDropDownMenu *)> &initializer)
{
	auto *row = AddRow(cvarName);
	if(row == nullptr)
		return nullptr;
	auto hDropDownMenu = CreateChild<WIDropDownMenu>();
	auto *pDropDownMenu = hDropDownMenu.get<WIDropDownMenu>();
	auto sz = GetSize();
	pDropDownMenu->SetSize(sz.x, sz.y);
	pDropDownMenu->SetAnchor(0.f, 0.f, 1.f, 1.f);
	pDropDownMenu->SetOptions(list);
	row->SetValue(0, name);
	if(initializer != nullptr)
		initializer(pDropDownMenu);
	if(!cvarName.empty()) {
		auto hOptions = GetHandle();
		pDropDownMenu->SelectOption(get_client_state()->GetConVarString(cvarName));
		pDropDownMenu->AddCallback("OnOptionSelected", FunctionCallback<void, uint32_t>::Create([hOptions, pDropDownMenu, cvarName](uint32_t optionIdx) mutable {
			if(!hOptions.IsValid())
				return;
			hOptions.get<WIOptionsList>()->m_updateCvars[cvarName] = pDropDownMenu->GetOptionValue(optionIdx);
		}));
	}
	row->InsertElement(1, pDropDownMenu);
	return pDropDownMenu;
}
pragma::gui::types::WIDropDownMenu *pragma::gui::types::WIOptionsList::AddDropDownMenu(const std::string &name, const std::unordered_map<std::string, std::string> &list, const std::string &cvarName)
{
	return AddDropDownMenu<const std::unordered_map<std::string, std::string> &>(name, list, cvarName, nullptr);
}
pragma::gui::types::WIDropDownMenu *pragma::gui::types::WIOptionsList::AddDropDownMenu(const std::string &name, const std::vector<std::string> &list, const std::string &cvarName) { return AddDropDownMenu<const std::vector<std::string> &>(name, list, cvarName, nullptr); }
pragma::gui::types::WIDropDownMenu *pragma::gui::types::WIOptionsList::AddDropDownMenu(const std::string &name, const std::function<void(WIDropDownMenu *)> &initializer, const std::string &cvarName)
{
	return AddDropDownMenu<const std::vector<std::string> &>(name, std::vector<std::string>({}), cvarName, initializer);
}
pragma::gui::types::WIDropDownMenu *pragma::gui::types::WIOptionsList::AddDropDownMenu(const std::string &name, const std::string &cvarName) { return AddDropDownMenu(name, nullptr, cvarName); }

std::unordered_map<std::string, std::string> &pragma::gui::types::WIOptionsList::GetUpdateConVars() { return m_updateCvars; }
void pragma::gui::types::WIOptionsList::RunUpdateConVars(bool bClear)
{
	auto *client = get_client_state();
	for(auto it = m_updateCvars.begin(); it != m_updateCvars.end(); ++it) {
		std::vector<std::string> argv = {it->second};
		client->RunConsoleCommand(it->first, argv);
	}
	if(bClear == true)
		m_updateCvars.clear();
	auto coreLayer = get_cengine()->GetCoreInputBindingLayer();
	for(char i = 0; i < 2; i++) {
		for(auto it = m_keyBindingsErase[i].begin(); it != m_keyBindingsErase[i].end(); ++it) {
			auto &cmd = it->first;
			auto &key = it->second;
			if(coreLayer)
				coreLayer->RemoveKeyMapping(CUInt16(key), cmd);
		}
		m_keyBindingsErase[i].clear();
		for(auto it = m_keyBindingsAdd[i].begin(); it != m_keyBindingsAdd[i].end(); ++it) {
			auto &cmd = it->first;
			if(coreLayer)
				coreLayer->AddKeyMapping(math::to_integral(it->second), cmd);
		}
		m_keyBindingsAdd[i].clear();
	}
}
pragma::gui::types::WITextEntry *pragma::gui::types::WIOptionsList::AddTextEntry(const std::string &name, const std::string &cvarName)
{
	auto *row = AddRow(cvarName);
	if(row == nullptr)
		return nullptr;
	auto hTextEntry = CreateChild<WITextEntry>();
	auto *pTextEntry = hTextEntry.get<WITextEntry>();
	auto sz = GetSize();
	pTextEntry->SetSize(sz.x, sz.y);
	pTextEntry->SetAnchor(0.f, 0.f, 1.f, 1.f);
	row->SetValue(0, name);
	if(!cvarName.empty()) {
		auto hOptions = GetHandle();
		pTextEntry->SetText(get_client_state()->GetConVarString(cvarName));
		pTextEntry->AddCallback("OnTextChanged", FunctionCallback<void, std::reference_wrapper<const string::Utf8String>, bool>::Create([hOptions, cvarName](std::reference_wrapper<const string::Utf8String> text, bool) mutable {
			if(!hOptions.IsValid())
				return;
			hOptions.get<WIOptionsList>()->m_updateCvars[cvarName] = text.get().cpp_str();
		}));
	}
	row->InsertElement(1, pTextEntry);
	return pTextEntry;
}
pragma::gui::types::WISlider *pragma::gui::types::WIOptionsList::AddSlider(const std::string &name, const std::function<void(WISlider *)> &initializer, const std::string &cvarName)
{
	auto *row = AddRow(cvarName);
	if(row == nullptr)
		return nullptr;
	auto hSlider = CreateChild<WISlider>();
	auto *pSlider = hSlider.get<WISlider>();
	auto sz = GetSize();
	pSlider->SetSize(sz.x, sz.y);
	pSlider->SetAnchor(0.f, 0.f, 1.f, 1.f);
	row->SetValue(0, name);
	if(initializer != nullptr)
		initializer(pSlider);
	if(!cvarName.empty()) {
		auto hOptions = GetHandle();
		pSlider->SetValue(get_client_state()->GetConVarFloat(cvarName));
		pSlider->AddCallback("OnChange", FunctionCallback<void, float, float>::Create([hOptions, cvarName](float, float value) mutable {
			if(!hOptions.IsValid())
				return;
			hOptions.get<WIOptionsList>()->m_updateCvars[cvarName] = std::to_string(value);
		}));
	}
	row->InsertElement(1, pSlider);
	return pSlider;
}
void pragma::gui::types::WIOptionsList::AddKeyBinding(const std::string &keyName, const std::string &cvarName)
{
	auto *row = AddRow(cvarName);
	if(row == nullptr)
		return;
	std::vector<platform::Key> mappedKeys {};
	get_cengine()->GetMappedKeys(cvarName, mappedKeys, 2u);
	auto key1 = (mappedKeys.size() > 0) ? mappedKeys.at(0) : static_cast<platform::Key>(-1);
	auto key2 = (mappedKeys.size() > 1) ? mappedKeys.at(1) : static_cast<platform::Key>(-1);
	row->SetValue(0, keyName);
	WIHandle hOptionsList = GetHandle();
	auto callback = [hOptionsList, cvarName](int entryId, WIHandle hKeyOther, platform::Key oldKey, platform::Key newKey) mutable {
		if(!hOptionsList.IsValid())
			return;
		auto *pOptionsList = hOptionsList.get<WIOptionsList>();
		if(oldKey != static_cast<platform::Key>(-1) && (!hKeyOther.IsValid() || (hKeyOther.get<WIKeyEntry>()->GetKey() != oldKey)))
			pOptionsList->m_keyBindingsErase[entryId][cvarName] = oldKey;
		pOptionsList->m_keyBindingsAdd[entryId][cvarName] = newKey;
	};

	auto &gui = WGUI::GetInstance();
	auto *pKey1 = gui.Create<WIKeyEntry>();
	auto *pKey2 = gui.Create<WIKeyEntry>();
	pKey1->SetKey(key1);
	pKey2->SetKey(key2);
	row->InsertElement(1, pKey1);
	pKey1->SetAutoAlignToParent(true);
	pKey1->AddCallback("OnKeyChanged", FunctionCallback<void, platform::Key, platform::Key>::Create(std::bind(callback, 0, pKey2->GetHandle(), std::placeholders::_1, std::placeholders::_2)));

	row->InsertElement(2, pKey2);
	pKey2->SetAutoAlignToParent(true);
	pKey2->AddCallback("OnKeyChanged", FunctionCallback<void, platform::Key, platform::Key>::Create(std::bind(callback, 1, pKey1->GetHandle(), std::placeholders::_1, std::placeholders::_2)));
}
