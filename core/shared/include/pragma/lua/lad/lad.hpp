/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */
#ifdef _MSC_VER
#ifndef __LUA_AUTO_DOC_LAD_HPP__
#define __LUA_AUTO_DOC_LAD_HPP__

#include "pragma/networkdefinitions.h"
#include <luabind/detail/type_info.hpp>
#include <util_pragma_doc.hpp>

namespace pragma::lua
{
	struct LuaOverloadInfo
	{
		std::vector<luabind::detail::TypeInfo> parameters {};
		std::vector<luabind::detail::TypeInfo> returnValues {};
		std::optional<pragma::doc::Source> source {};
		std::optional<std::vector<std::string>> namedParameters {};
	};

	struct LuaMethodInfo
	{
		std::string name;
		std::vector<LuaOverloadInfo> overloads;
	};

	struct LuaClassInfo
	{
		using EnumSet = std::unordered_map<std::string,int32_t>;
		luabind::detail::class_rep *classRep = nullptr;
		std::string name;
		std::vector<LuaMethodInfo> methods;
		std::vector<std::string> attributes;
		std::vector<luabind::detail::class_rep*> bases;
		std::unordered_map<std::string,EnumSet> enumSets;
	};

	struct ClassRegInfo
	{
		std::string path;
		LuaClassInfo classInfo;
		bool cppDefined = true;
	};
};

#endif
#endif
