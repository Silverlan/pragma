/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/classes/ldef_entity.h"

void LuaEntityManager::RegisterEntity(std::string className, luabind::object &o, const std::vector<pragma::ComponentId> &components)
{
	EntityInfo entInfo {};
	entInfo.classObject = o;
	entInfo.components = components;
	ustring::to_lower(className);
	m_ents[className] = std::move(entInfo);
	//m_ents.insert(std::unordered_map<std::string,luabind::object>::value_type(className,o));
}

luabind::object *LuaEntityManager::GetClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_ents.find(className);
	if(it == m_ents.end())
		return nullptr;
	return &it->second.classObject;
}

LuaEntityManager::EntityInfo *LuaEntityManager::GetEntityInfo(std::string className)
{
	ustring::to_lower(className);
	auto it = m_ents.find(className);
	if(it == m_ents.end())
		return nullptr;
	return &it->second;
}

void LuaEntityManager::RegisterComponent(std::string className, luabind::object &o, pragma::ComponentId componentId)
{
	ustring::to_lower(className);
	m_components[className] = {o,componentId};
}
std::optional<pragma::ComponentId> LuaEntityManager::FindComponentId(const luabind::object &o) const {
	auto it = std::find_if(m_components.begin(), m_components.end(), [&o](const std::pair < std::string, std::pair<luabind::object, pragma::ComponentId>> &pair) {
		return pair.second.first == o;
	});
	return (it != m_components.end()) ? it->second.second : std::optional<pragma::ComponentId> {};
}
const luabind::object *LuaEntityManager::FindClassObject(pragma::ComponentId componentId) const {
	auto it = std::find_if(m_components.begin(), m_components.end(), [componentId](const std::pair<std::string, std::pair<luabind::object, pragma::ComponentId>> &pair) { return pair.second.second == componentId; });
	return (it != m_components.end()) ? &it->second.first : nullptr;
}
luabind::object *LuaEntityManager::GetComponentClassObject(std::string className)
{
	ustring::to_lower(className);
	auto it = m_components.find(className);
	if(it == m_components.end())
		return nullptr;
	return &it->second.first;
}
