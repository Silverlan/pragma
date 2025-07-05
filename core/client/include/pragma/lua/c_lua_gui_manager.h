// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_LUA_GUI_MANAGER_H__
#define __C_LUA_GUI_MANAGER_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT LuaGUIManager {
  private:
	std::unordered_map<std::string, luabind::object> m_guiElements;
  public:
	void RegisterGUIElement(std::string className, luabind::object &o);
	luabind::object *GetClassObject(std::string className);
};
#pragma warning(pop)

#endif
