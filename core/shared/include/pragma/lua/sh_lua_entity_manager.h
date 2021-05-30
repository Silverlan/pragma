/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __SH_LUA_ENTITY_MANAGER_H__
#define __SH_LUA_ENTITY_MANAGER_H__

#include "pragma/networkdefinitions.h"
#include <pragma/lua/luaapi.h>
#include <unordered_map>

class DLLNETWORK LuaEntityManager
{
private:
	std::unordered_map<std::string,luabind::object> m_ents;
	std::unordered_map<std::string,luabind::object> m_components;
public:
	void RegisterEntity(std::string className,luabind::object &o);
	luabind::object *GetClassObject(std::string className);

	void RegisterComponent(std::string className,luabind::object &o);
	luabind::object *GetComponentClassObject(std::string className);
};

#endif