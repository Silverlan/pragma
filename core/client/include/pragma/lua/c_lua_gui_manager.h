/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_LUA_GUI_MANAGER_H__
#define __C_LUA_GUI_MANAGER_H__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT LuaGUIManager
{
private:
	std::unordered_map<std::string,luabind::object> m_guiElements;
public:
	void RegisterGUIElement(std::string className,luabind::object &o);
	luabind::object *GetClassObject(std::string className);
};
#pragma warning(pop)

#endif