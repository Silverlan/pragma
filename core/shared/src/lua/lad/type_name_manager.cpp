/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/lad/type_name_manager.hpp"

#pragma comment(lib,"Psapi.lib")

using namespace pragma::lua;

void TypeNameManager::RegisterType(const std::type_info &typeInfo,const luabind::detail::class_rep &luaClassDef)
{
	auto it = m_types.find(&typeInfo);
	if(it != m_types.end())
		return;
	TypeInfo ti {};
	ti.hash = typeInfo.hash_code();
	ti.name = typeInfo.name();

	auto simplifiedName = ti.name;
	ustring::replace(simplifiedName,"struct ","");
	ustring::replace(simplifiedName,"class ","");
	ti.simplifiedName = std::move(simplifiedName);

	ti.decoratedName = typeInfo.raw_name();
	ti.luaClassDef = &luaClassDef;
	ti.luaName = luaClassDef.name();
	if(!ti.decoratedName.empty())
		ti.decoratedName.erase(ti.decoratedName.begin());
	ti.decoratedName = "??_R0" +ti.decoratedName +"@8"; // This will ensure that the name matches the name as it appears in the PDB files
	m_types.insert(std::make_pair(&typeInfo,new TypeInfo{std::move(ti)}));
}
const TypeNameManager::TypeInfo *TypeNameManager::GetTypeInfo(const std::type_info *ti) const
{
	auto it = m_types.find(ti);
	return (it != m_types.end()) ? it->second : nullptr;
}
std::optional<const std::string_view> TypeNameManager::TranslateType(const std::string &type) const
{
	static const std::unordered_map<std::string,std::string> defaultTranslations {
		{"luabind::adl::object","any"},
		{"std::basic_string<char,std::char_traits<char>,std::allocator<char> >","string"},
		{"unsigned __int64","uint64"},
		{"bool","bool"},
		{"luabind::adl::argument","any"},
		{"int","int32"},
		{"unsigned int","uint32"},
		{"__int64","int64"},
		{"char","int8"},
		{"signed char","int8"},
		{"unsigned char","uint8"},
		{"unsigned short","uint16"},
		{"short","int16"},
		{"custom [float]","float"},
		{"std::string const&","string"},
		{"std::string","string"},
		{"custom [unsigned __int64]","uint64"},

		// Special cases
		{"luabind::argument const&","any"}, // Used by "class_info"
		{"class_info_data","class_info_data"}, // Used by "class_info"
		{"lua_State*","lua_State"}, // Used by "class_names"
		{"luabind::object","any"} // Used by "class_names"

	};
	auto itDef = defaultTranslations.find(type);
	if(itDef != defaultTranslations.end())
		return itDef->second;
	auto it = m_assignedTypes.find(type);
	if(it == m_assignedTypes.end() || it->second == nullptr)
		return {};
	return const_cast<TypeNameManager*>(this)->m_types[it->second]->luaName;
}
void TypeNameManager::AssignType(const std::string &name)
{
	auto it = m_assignedTypes.find(name);
	if(it != m_assignedTypes.end())
		return;
	auto maxSim = std::numeric_limits<double>::lowest();
	const type_info *bestCandidate = nullptr;
	for(auto &pair : m_types)
	{
		auto sim = ustring::calc_levenshtein_similarity(name,pair.second->name);
		if(sim <= maxSim)
			continue;
		maxSim = sim;
		bestCandidate = pair.first;
	}
	if(bestCandidate == nullptr)
		return;
	auto &typeInfo = *m_types[bestCandidate];
	if(maxSim > typeInfo.bestMatchScore)
	{
		if(typeInfo.bestMatch.has_value())
			m_assignedTypes[*typeInfo.bestMatch] = nullptr;
		typeInfo.bestMatch = name;
		typeInfo.bestMatchScore = maxSim;
		m_assignedTypes[name] = bestCandidate;
	}
	else
		m_assignedTypes[name] = nullptr;
}
