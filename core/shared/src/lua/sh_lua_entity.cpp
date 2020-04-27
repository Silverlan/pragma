/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/ldef_entity.h"

void LuaEntityManager::RegisterEntity(std::string className,luabind::object &o)
{
	ustring::to_lower(className);
	m_ents[className] = o;
	//m_ents.insert(std::unordered_map<std::string,luabind::object>::value_type(className,o));
}

luabind::object *LuaEntityManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_ents.find(className);
	if(it == m_ents.end())
		return nullptr;
	return &it->second;
}

void LuaEntityManager::RegisterComponent(std::string className,luabind::object &o)
{
	ustring::to_lower(className);
	m_components[className] = o;
}
luabind::object *LuaEntityManager::GetComponentClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_components.find(className);
	if(it == m_components.end())
		return nullptr;
	return &it->second;
}
