// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;

import :scripting.lua.gui_manager;

void LuaGUIManager::RegisterGUIElement(std::string className, luabind::object &o)
{
	ustring::to_lower(className);
	m_guiElements[className] = o;
	//m_guiElements.insert(std::unordered_map<std::string,luabind::object>::value_type(className,o));
}

luabind::object *LuaGUIManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_guiElements.find(className);
	if(it == m_guiElements.end())
		return nullptr;
	return &it->second;
}
