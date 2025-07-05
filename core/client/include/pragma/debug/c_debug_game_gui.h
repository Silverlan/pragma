// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_DEBUG_GAME_GUI_H__
#define __C_DEBUG_GAME_GUI_H__

#include "pragma/clientdefinitions.h"
#include <sharedutils/functioncallback.h>
#include <wgui/wihandle.h>
#include <wgui/wibase.h>
#include <unordered_map>

struct DLLCLIENT DebugGameGUI {
  private:
	std::unordered_map<std::string, CallbackHandle> m_callbacks;
	std::vector<std::function<void(void)>> m_callOnRemove;
	std::unordered_map<uint32_t, std::shared_ptr<void>> m_userData;
	std::function<WIHandle(void)> m_guiFactory;
	CallbackHandle m_initCallback;
	WIHandle m_guiElement;
	void Initialize();
  public:
	DebugGameGUI(const std::function<WIHandle(void)> &guiFactory);
	~DebugGameGUI();
	void AddCallback(const std::string &identifier, const CallbackHandle &hCallback);
	void CallOnRemove(const std::function<void(void)> &f);
	void SetUserData(uint32_t idx, const std::shared_ptr<void> &data);
	std::shared_ptr<void> GetUserData(uint32_t idx) const;
	WIBase *GetGUIElement();
};

#endif
