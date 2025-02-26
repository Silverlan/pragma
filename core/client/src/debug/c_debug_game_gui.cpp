/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/game/c_game.h"
#include <wgui/wibase.h>

extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

DebugGameGUI::DebugGameGUI(const std::function<WIHandle(void)> &guiFactory) : m_guiFactory(guiFactory)
{
	m_initCallback = client->AddCallback("OnGameStart", FunctionCallback<void, CGame *>::Create([this](CGame *) { Initialize(); }));
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

WIBase *DebugGameGUI::GetGUIElement() { return m_guiElement.get(); }

void DebugGameGUI::Initialize()
{
	if(c_game == nullptr)
		return;
	if(m_guiFactory == nullptr)
		return;
	auto hEl = m_guiFactory();
	AddCallback("OnGameEnd", FunctionCallback<>::Create([hEl]() {
		if(hEl.IsValid())
			const_cast<WIBase *>(hEl.get())->Remove();
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
	c_game->AddCallback(identifier, hCallback);
}
