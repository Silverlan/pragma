// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :debug.game_ui;
import :client_state;
import :game;

DebugGameGUI::DebugGameGUI(const std::function<pragma::gui::WIHandle(void)> &guiFactory) : m_guiFactory(guiFactory)
{
	m_initCallback = pragma::get_client_state()->AddCallback("OnGameStart", FunctionCallback<void, pragma::CGame *>::Create([this](pragma::CGame *) { Initialize(); }));
	Initialize();
}

DebugGameGUI::~DebugGameGUI()
{
	if(m_initCallback.IsValid())
		m_initCallback.Remove();
	if(m_guiElement.IsValid())
		m_guiElement->Remove();
	for(auto &it : m_callbacks) {
		if(it.second.IsValid())
			it.second.Remove();
	}
	for(auto &f : m_callOnRemove)
		f();
}

void DebugGameGUI::SetUserData(uint32_t idx, const std::shared_ptr<void> &data) { m_userData[idx] = data; }
std::shared_ptr<void> DebugGameGUI::GetUserData(uint32_t idx) const
{
	auto it = m_userData.find(idx);
	if(it == m_userData.end())
		return nullptr;
	return it->second;
}

pragma::gui::types::WIBase *DebugGameGUI::GetGUIElement() { return m_guiElement.get(); }

void DebugGameGUI::Initialize()
{
	if(pragma::get_cgame() == nullptr)
		return;
	if(m_guiFactory == nullptr)
		return;
	auto hEl = m_guiFactory();
	AddCallback("OnGameEnd", FunctionCallback<>::Create([hEl]() {
		if(hEl.IsValid())
			const_cast<pragma::gui::types::WIBase *>(hEl.get())->Remove();
	}));
	m_guiElement = hEl;
}

void DebugGameGUI::CallOnRemove(const std::function<void(void)> &f) { m_callOnRemove.push_back(f); }

void DebugGameGUI::AddCallback(const std::string &identifier, const CallbackHandle &hCallback)
{
	auto it = m_callbacks.find(identifier);
	if(it != m_callbacks.end() && it->second.IsValid())
		it->second.Remove();
	m_callbacks[identifier] = hCallback;
	pragma::get_cgame()->AddCallback(identifier, hCallback);
}
