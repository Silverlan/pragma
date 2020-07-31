/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/lua/class_manager.hpp"

#pragma optimize("",off)
pragma::lua::ClassManager::ClassManager(lua_State &l)
	: m_luaState{l}
{}
pragma::lua::ClassManager::ClassInfo *pragma::lua::ClassManager::FindClassInfo(const ClassRef &classRef)
{
	if(classRef.className.has_value())
	{
		auto it = m_classNameToClassIndex.find(*classRef.className);
		return (it != m_classNameToClassIndex.end()) ? &m_classes.at(it->second) : nullptr;
	}
	if(classRef.classObject.has_value() == false)
		return nullptr;
	auto it = std::find_if(m_classes.begin(),m_classes.end(),[&classRef](const ClassInfo &classInfo) {
		return *classRef.classObject == classInfo.classObject;
	});
	return (it != m_classes.end()) ? &*it : nullptr;
}
const pragma::lua::ClassManager::ClassInfo *pragma::lua::ClassManager::FindClassInfo(const ClassRef &classRef) const
{
	return const_cast<ClassManager*>(this)->FindClassInfo(classRef);
}
void pragma::lua::ClassManager::RegisterClass(const std::string &className,luabind::object oClass)
{
	/*
	// Obsolete? Lua-registered classes don't seem to be a part of the luabind registry
	auto *reg = luabind::detail::class_registry::get_registry(&m_luaState);
	if(reg == nullptr)
		return;
	auto &classes = reg->get_classes();
	auto it = std::find_if(classes.begin(),classes.end(),[&className](const std::pair<luabind::type_id,luabind::detail::class_rep*> &pair) {
		return pair.second->name() == className;
	});
	if(it == classes.end())
		return;*/

	auto it = std::find_if(m_classes.begin(),m_classes.end(),[&className](const ClassInfo &classInfo) {
		return classInfo.className == className;
	});
	if(it != m_classes.end())
		return;
	if(m_classes.size() == m_classes.capacity())
		m_classes.reserve(m_classes.size() +100);
	m_classes.push_back({});
	auto &classInfo = m_classes.back();
	classInfo.classObject = oClass;
	classInfo.className = className;

	m_classNameToClassIndex[className] = m_classes.size() -1u;
}
bool pragma::lua::ClassManager::IsClassRegistered(const ClassRef &classRef) const
{
	auto *classInfo = FindClassInfo(classRef);
	return classInfo;
}
bool pragma::lua::ClassManager::IsClassMethodDefined(const ClassRef &classRef,const std::string &methodName) const
{
	return false;
	// Obsolete?
#if 0
	auto *classInfo = FindClassInfo(classRef);
	if(classInfo == nullptr)
		return false;
	auto *l = &m_luaState;
	auto &reg = *classInfo->class_rep;
	reg.get_table(l);
	luabind::object table{luabind::from_stack(l,-1)};
	lua_pop(l,1);

	std::size_t index = 1;
	for(luabind::iterator i{table}, e; i != e; ++i)
	{
		std::string key;
		auto lkey = i.key();
		lkey.push(l);
		if(Lua::IsString(l,-1))
			key = Lua::CheckString(l,-1);
		Lua::Pop(l,1);

		auto type = luabind::type(*i);
		if(type == LUA_TFUNCTION && key == methodName)
			return true;
	}
	return false;
#endif
}
#pragma optimize("",on)
